#pragma once

#include "../config/ProfileManager.h"
#include "../utils/VUBuffer.h"
#include <string>

void ShowEQSwitchWindow(ProfileManager& profileManager,
                        VUBuffer& vuBuffer,
                        bool* p_exit = nullptr, float main_scale = 1.0f);

int findCurrentProfileIndex(const std::vector<Profile>& profiles, const std::string& currentProfile);                   
