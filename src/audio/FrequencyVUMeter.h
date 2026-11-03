#pragma once

#include <array>
#include <vector>
#include <chrono>

#include "utils\kissfft\kiss_fft.h"
#include "utils\kissfft\kiss_fftr.h"

class FrequencyVUMeter
{
public:
    static constexpr int NUM_BANDS = 15;

    static constexpr std::array<const char *, NUM_BANDS> bandLabels = {
        "25", "40", "63", "100", "160",
        "250", "400", "630", "1k", "1.6k",
        "2.5k", "4k", "6.3k", "10k", "16k"};

    FrequencyVUMeter(int fftSize = 8192, float decayRate = 0.05f, int sampleRate = 48000);
    ~FrequencyVUMeter();

    void processAudio(const float *samples, size_t sampleCount, int sampleRate);
    void processAudio(const float *interleavedSamples, size_t frameCount, int sampleRate, int channelCount);
    const std::vector<float> &getBandLevels() const { return bandLevels; }
    void resetLevels();
    std::vector<float> getNormalizedBandLevels() const;
    std::vector<std::pair<float, float>> getBandRanges(int sampleRate) const;

private:
    int fftSize;
    std::vector<float> bandLevels;
    float decayRate;
    std::chrono::steady_clock::time_point lastUpdateTime;
    kiss_fftr_cfg fftCfg;
    std::vector<kiss_fft_cpx> freqData;
    std::vector<float> magnitudes;
    std::vector<float> windowedSamples;
    std::vector<float> windowCoefficients;
    float windowSum;

    void computeBands(const kiss_fft_scalar *interleavedSamples, int sampleRate, int channelCount);
};
