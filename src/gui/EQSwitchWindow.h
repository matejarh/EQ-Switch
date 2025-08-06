#pragma once

#include "../config/ProfileManager.h"
#include "../utils/VUBuffer.h"
#include "../utils/EqualizerAPOManager.h"
#include <string>
/* #include <d3d11.h> */

void ShowEQSwitchWindow(ProfileManager& profileManager,
                        VUBuffer& vuBuffer,
                        bool* p_exit = nullptr, 
                        float main_scale = 1.0f, 
                        EqualizerAPOManager& apoManager = EqualizerAPOManager::getInstance()/* ,
                        ID3D11ShaderResourceView* iconTexture = nullptr */
                        );

int findCurrentProfileIndex(const std::vector<Profile>& profiles, const std::string& currentProfile);           
