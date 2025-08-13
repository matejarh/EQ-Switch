#pragma once

#include "imgui.h"

// Global pointer (or you can make it part of a class/namespace if preferred)
extern ImFont* g_SmallFont;
extern ImFont* g_SmallFontMono;
extern ImFont* g_unicodeFont;
extern ImFont* g_SmallVuLabels;

// Call this once after ImGui context is created
void LoadFonts(float baseFontSize = 20.0f);
