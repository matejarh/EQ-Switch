#pragma once

#include <thread>
#include <atomic>
#include <vector>

#include "audio/VUBuffer.h"
#include "audio/FrequencyVUMeter.h"

class AudioCapture
{
public:
    AudioCapture(VUBuffer &buffer, int sampleRate, int fftSize = 8192, float decayTimeSeconds = 0.43f);
    ~AudioCapture();

    void start();
    void stop();

    const FrequencyVUMeter& getFrequencyVUMeter() const { return frequencyVUMeter; }
    int getSampleRate() const { return sampleRate.load(); }

private:
    void captureLoop();

    VUBuffer &vuBuffer;
    FrequencyVUMeter frequencyVUMeter;
    std::atomic<int> sampleRate;
    int fftSize;

    std::atomic<bool> running;
    std::thread captureThread;

    std::vector<float> sampleBuffer;
    size_t sampleWritePos = 0;
    std::vector<float> vuLevels;
};
