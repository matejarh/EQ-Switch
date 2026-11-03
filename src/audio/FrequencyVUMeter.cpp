#include <cmath>
#include <algorithm>
#include <iostream>

#include "audio/FrequencyVUMeter.h"

FrequencyVUMeter::FrequencyVUMeter(int fftSize, float decayTimeSeconds, int sampleRate)
    : fftSize(fftSize),
      bandLevels(NUM_BANDS, 0.0f),
            decayTimeSeconds(decayTimeSeconds),
      lastUpdateTime(std::chrono::steady_clock::now()),
      freqData(fftSize / 2 + 1),
      magnitudes(fftSize / 2 + 1),
            windowedSamples(fftSize),
            windowCoefficients(fftSize),
            windowSum(0.0f)
{
        constexpr float pi = 3.14159265358979323846f;
        for (int i = 0; i < fftSize; ++i)
        {
                windowCoefficients[i] = 0.5f * (1.0f - std::cos(2.0f * pi * i / (fftSize - 1)));
                windowSum += windowCoefficients[i];
        }

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
    processAudio(samples, sampleCount, sampleRate, 1);
}

void FrequencyVUMeter::processAudio(const float *interleavedSamples, size_t frameCount, int sampleRate, int channelCount)
{
    if (frameCount < static_cast<size_t>(fftSize) || channelCount <= 0)
    {
        // Not enough samples, ignore or buffer externally before calling
        return;
    }

    auto now = std::chrono::steady_clock::now();
    float elapsedSeconds = std::chrono::duration<float>(now - lastUpdateTime).count();
    lastUpdateTime = now;
    computeBands(interleavedSamples, sampleRate, channelCount, elapsedSeconds);
}

void FrequencyVUMeter::resetLevels()
{
    std::fill(bandLevels.begin(), bandLevels.end(), 0.0f);
    lastUpdateTime = std::chrono::steady_clock::now();
}

void FrequencyVUMeter::computeBands(const kiss_fft_scalar *interleavedSamples, int sampleRate, int channelCount, float elapsedSeconds)
{
    // Frequency band edges (Hz)
    constexpr std::array<float, NUM_BANDS + 1> bandEdges = {
        25.0f, 40.0f, 63.0f, 100.0f, 160.0f,
        250.0f, 400.0f, 630.0f, 1000.0f, 1600.0f,
        2500.0f, 4000.0f, 6300.0f, 10000.0f, 16000.0f, 20000.0f};

    std::array<float, NUM_BANDS> newLevels = {0};

    for (int channel = 0; channel < channelCount; ++channel)
    {
        for (int i = 0; i < fftSize; ++i)
            windowedSamples[i] = interleavedSamples[static_cast<size_t>(i) * channelCount + channel] * windowCoefficients[i];

        kiss_fftr(fftCfg, windowedSamples.data(), freqData.data());

        for (int i = 0; i < fftSize / 2 + 1; ++i)
        {
            magnitudes[i] = std::sqrt(freqData[i].r * freqData[i].r + freqData[i].i * freqData[i].i);
        }

        for (size_t b = 0; b < NUM_BANDS; ++b)
        {
            float startFreq = bandEdges[b];
            float endFreq = bandEdges[b + 1];
            float nyquist = sampleRate * 0.5f;
            if (startFreq >= nyquist)
                continue;

            size_t startBin = static_cast<size_t>(startFreq * fftSize / sampleRate);
            size_t endBin = static_cast<size_t>(std::min(endFreq, nyquist) * fftSize / sampleRate);
            startBin = std::min(startBin, magnitudes.size() - 1);
            endBin = std::min(endBin, magnitudes.size() - 1);

            float peakMagnitude = 0.0f;
            for (size_t bin = startBin; bin <= endBin; ++bin)
                peakMagnitude = std::max(peakMagnitude, magnitudes[bin]);

            float peakAmplitude = (windowSum > 0.0f) ? (2.0f * peakMagnitude / windowSum) : 0.0f;
            float levelDb = 20.0f * std::log10(std::max(peakAmplitude, 1.0e-6f));
            constexpr float meterFloorDb = -60.0f;
            float level = std::clamp((levelDb - meterFloorDb) / -meterFloorDb, 0.0f, 1.0f);
            newLevels[b] = std::max(newLevels[b], level);
        }
    }

    float decayAmount = elapsedSeconds / std::max(decayTimeSeconds, 1.0e-6f);
    for (size_t i = 0; i < NUM_BANDS; ++i)
    {
        bandLevels[i] = std::max(newLevels[i], std::max(0.0f, bandLevels[i] - decayAmount));
    }
}
