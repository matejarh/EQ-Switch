#pragma once
#include "imgui.h"
#include "ProfileManager.h"

void ProfilesSection(ProfileManager &profileManager, std::string &currentProfileOut, int &selectedProfileOut, bool *p_exit = nullptr);