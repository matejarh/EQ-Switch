// AudioCapture.h
#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include "../utils/VUBuffer.h"

class AudioCapture {
public:
    AudioCapture(VUBuffer& buffer);
    ~AudioCapture();
    void start();
    void stop();

private:
    void captureLoop();
    VUBuffer& vuBuffer;
    std::thread captureThread;
    std::atomic<bool> running;
};
