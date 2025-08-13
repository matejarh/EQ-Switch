#pragma once

#include "../config/ProfileManager.h"
#include "../utils/VUBuffer.h"
#include "../utils/EqualizerAPOManager.h"
#include "../audio/FrequencyVUMeter.h"
#include <string>
#include <AudioCapture.h>

void ShowEQSwitchWindow(ProfileManager &profileManager,
                        VUBuffer &vuBuffer,
                        bool *p_exit, float main_scale,
                        EqualizerAPOManager &apoManager,
                        FrequencyVUMeter &frequencyVumeter,
                        AudioCapture &audioCapture, int sampleRate
                    );

int findCurrentProfileIndex(const std::vector<Profile>& profiles, const std::string& currentProfile);           
