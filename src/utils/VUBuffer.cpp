#include "VUBuffer.h"
#include <algorithm>
#include <mutex>

VUBuffer::VUBuffer(int numChannels)
    : buffer(numChannels, 0.0f)
{
}

void VUBuffer::setVU(int channel, float value)
{
    std::lock_guard<std::mutex> lock(vuMutex);
    if (channel >= 0 && channel < buffer.size())
    {
        buffer[channel] = std::clamp(value, 0.0f, 1.0f);
    }
}

void VUBuffer::setChannelVU(int channelIndex, float value) {
    std::lock_guard<std::mutex> lock(vuMutex);
    if (channelIndex >= 0 && channelIndex < static_cast<int>(buffer.size())) {
        buffer[channelIndex] = value;
    }
}

std::vector<float> VUBuffer::getAll() const
{
    std::lock_guard<std::mutex> lock(vuMutex);
    return buffer;
}

float VUBuffer::get(int channel)
{
    std::lock_guard<std::mutex> lock(vuMutex);
    if (channel >= 0 && channel < buffer.size())
        return buffer[channel];
    return 0.0f;
}

int VUBuffer::getChannelCount() const
{
    std::lock_guard<std::mutex> lock(vuMutex);
    return static_cast<int>(buffer.size());
}
