#pragma once

#include <imgui.h>
#include "utils/fonts.h"
#include "audio/VUBuffer.h"
#include "utils/ChannelLabels.h"
#include "audio/FrequencyVUMeter.h"

enum class VuMeterMode
{
    ProgressBar,
    LedHorizontal,
    LedVertical
};

class VuMeters
{
public:
    void VuMeterProgresBar(VUBuffer &vuBuffer, float main_scale);
    void LedVuMeterHorizontal(VUBuffer &vuBuffer, float main_scale);
    void LedVuMeter(VUBuffer &vuBuffer, float main_scale);
    void DrawFrequencyVUMeter(const std::vector<float> &levels, const std::vector<std::pair<float, float>> &bands, float main_scale, FrequencyVUMeter& frequencyVumeter, bool rightAlign);
};