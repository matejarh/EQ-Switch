#pragma once
#include <imgui.h>
#include "utils/EqualizerAPOManager.h"

// Footer height measured on the last frame, including bottom window padding
extern float g_footerHeight;

void Footer(EqualizerAPOManager &apoManager);