#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include "VUBuffer.h"
#include "FrequencyVUMeter.h"

class AudioCapture
{
public:
    AudioCapture(VUBuffer &buffer, int sampleRate, int fftSize = 2048);
    ~AudioCapture();

    void start();
    void stop();

    const FrequencyVUMeter& getFrequencyVUMeter() const { return frequencyVUMeter; }

private:
    void captureLoop();

    VUBuffer &vuBuffer;
    FrequencyVUMeter frequencyVUMeter;
    int sampleRate;
    int fftSize;

    std::atomic<bool> running;
    std::thread captureThread;

    std::vector<float> sampleBuffer;
    size_t sampleWritePos = 0;
};
