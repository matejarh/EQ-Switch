#pragma once
#include <imgui.h>

#include "config/ProfileManager.h"
#include "utils/EqualizerAPOManager.h"

void ProfilesSection(ProfileManager &profileManager,
                     std::string &currentProfileOut, 
                     int &selectedProfileOut, 
                     bool *p_exit = nullptr, 
                     EqualizerAPOManager &apoManager = EqualizerAPOManager::getInstance());