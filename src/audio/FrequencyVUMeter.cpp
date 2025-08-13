#include "FrequencyVUMeter.h"
#include <cmath>
#include <algorithm>
#include <iostream>

FrequencyVUMeter::FrequencyVUMeter(int fftSize, float decayRate, int sampleRate)
    : fftSize(fftSize),
      bandLevels(NUM_BANDS, 0.0f),
      decayRate(decayRate),
      lastUpdateTime(std::chrono::steady_clock::now())
{
    fftCfg = kiss_fftr_alloc(fftSize, 0, nullptr, nullptr);
    if (!fftCfg)
    {
        throw std::runtime_error("Failed to allocate kissfft config");
    }
}

FrequencyVUMeter::~FrequencyVUMeter()
{
    if (fftCfg)
    {
        free(fftCfg);
        fftCfg = nullptr;
    }
}

std::vector<float> FrequencyVUMeter::getNormalizedBandLevels() const
{
    // Normalize each band's amplitude to 0–1
    std::vector<float> normalized;
    normalized.reserve(bandLevels.size());

    float maxLevel = *std::max_element(bandLevels.begin(), bandLevels.end());
    if (maxLevel <= 0.0001f)
        maxLevel = 1.0f; // avoid divide by zero

    for (float level : bandLevels)
        normalized.push_back(level / maxLevel);

    return normalized;
}

std::vector<std::pair<float, float>> FrequencyVUMeter::getBandRanges(int sampleRate) const
{
    std::vector<std::pair<float, float>> ranges;
    ranges.reserve(bandLevels.size());

    // Simple equal division of frequency spectrum
    float nyquist = sampleRate / 2.0f;
    float bandWidth = nyquist / bandLevels.size();

    for (size_t i = 0; i < bandLevels.size(); i++)
    {
        float startFreq = i * bandWidth;
        float endFreq = (i + 1) * bandWidth;
        ranges.push_back({startFreq, endFreq});
    }
    return ranges;
}

void FrequencyVUMeter::processAudio(const float *samples, size_t sampleCount, int sampleRate)
{
    if (sampleCount < fftSize)
    {
        // Not enough samples, ignore or buffer externally before calling
        return;
    }

    computeBands(samples, sampleRate);
}

void FrequencyVUMeter::resetLevels()
{
    std::fill(bandLevels.begin(), bandLevels.end(), 0.0f);
}

void FrequencyVUMeter::computeBands(const kiss_fft_scalar *timeData, int sampleRate)
{
    std::vector<kiss_fft_cpx> freqData(fftSize / 2 + 1);

    // Run FFT
    kiss_fftr(fftCfg, timeData, freqData.data());

    // Compute magnitudes
    std::vector<float> magnitudes(fftSize / 2 + 1);
    for (int i = 0; i < fftSize / 2 + 1; ++i)
    {
        magnitudes[i] = std::sqrt(freqData[i].r * freqData[i].r + freqData[i].i * freqData[i].i);
    }

    // Frequency band edges (Hz)
    constexpr std::array<float, NUM_BANDS + 1> bandEdges = {
        25.0f, 40.0f, 63.0f, 100.0f, 160.0f,
        250.0f, 400.0f, 630.0f, 1000.0f, 1600.0f,
        2500.0f, 4000.0f, 6300.0f, 10000.0f, 16000.0f, 20000.0f};

    std::array<float, NUM_BANDS> newLevels = {0};

    // Map FFT bins to bands and calculate average magnitude per band
    for (size_t b = 0; b < NUM_BANDS; ++b)
    {
        float startFreq = bandEdges[b];
        float endFreq = bandEdges[b + 1];

        size_t startBin = static_cast<size_t>(startFreq * fftSize / sampleRate);
        size_t endBin = static_cast<size_t>(endFreq * fftSize / sampleRate);

        startBin = std::min(startBin, magnitudes.size() - 1);
        endBin = std::min(endBin, magnitudes.size() - 1);

        float sum = 0.0f;
        size_t count = 0;
        for (size_t bin = startBin; bin <= endBin; ++bin)
        {
            sum += magnitudes[bin];
            ++count;
        }

        // Average magnitude
        float linearLevel = (count > 0) ? (sum / count) : 0.0f;

        constexpr float expectedMaxAmplitude = 50.0f;
        float normalized = std::clamp(linearLevel / expectedMaxAmplitude, 0.0f, 1.0f);

        float whispers = 0.005f; // floor for whispers
        float highLevels = 10.0f; // contrast factor for mid-to-high levels

        float scaledLevel = std::log10f(1.0f + highLevels * normalized);
        scaledLevel = std::clamp(scaledLevel, whispers, 1.0f); // ensures bottom LEDs light up even for tiny signals

        // Remove floor a after clamping to 0..1
        scaledLevel = (scaledLevel - whispers) / (1.0f - whispers);

        newLevels[b] = scaledLevel;
    }

    // Smooth / decay
    for (size_t i = 0; i < NUM_BANDS; ++i)
    {
        if (newLevels[i] > bandLevels[i])
            bandLevels[i] = newLevels[i]; // instant rise
        else
            bandLevels[i] = std::max(0.0f, bandLevels[i] - decayRate); // gradual fall
    }
}
