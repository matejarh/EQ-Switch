#pragma once
#include <vector>
#include <mutex>

class VUBuffer
{
public:
    VUBuffer(int numChannels);

    void setVU(int channel, float value);     // Set smoothed VU value (from audio capture)
    void setChannelVU(int channelIndex, float value);
    std::vector<float> getAll() const;        // Get current VU values for all channels
    float get(int channel);                   // Get single channel's VU
    int getChannelCount() const;

private:
    mutable std::mutex vuMutex;
    std::vector<float> buffer;
};
