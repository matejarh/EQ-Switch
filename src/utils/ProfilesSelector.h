#pragma once
#include "imgui.h"
#include "ProfileManager.h"
#include "EqualizerAPOManager.h"

void ProfilesSection(ProfileManager &profileManager,
                     std::string &currentProfileOut, 
                     int &selectedProfileOut, 
                     bool *p_exit = nullptr, 
                     EqualizerAPOManager &apoManager = EqualizerAPOManager::getInstance());