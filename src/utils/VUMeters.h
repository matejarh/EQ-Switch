#pragma once

#include "VUBuffer.h"
#include "ChannelLabels.h"
#include <imgui.h>
#include "fonts.h"

class VuMeters
{
public:
    void VuMeterProgresBar(VUBuffer &vuBuffer, float main_scale);
   
    void LedVuMeterHorizontal(VUBuffer &vuBuffer, float main_scale);
    
    void LedVuMeter(VUBuffer &vuBuffer, float main_scale);
    
};