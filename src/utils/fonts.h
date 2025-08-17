#pragma once

#include "imgui.h"

// Global pointer (or you can make it part of a class/namespace if preferred)
extern ImFont* g_DefaultFont;
extern ImFont* g_BigFont;
extern ImFont* g_SmallFont;
extern ImFont* g_SmallerFont;
extern ImFont* g_SmallFontMono;
extern ImFont* g_LabelFont;
extern ImFont* g_unicodeFont;
extern ImFont* g_SmallVuLabels;
extern ImFont* g_fontAwesome;

// Call this once after ImGui context is created
void LoadFonts(float baseFontSize = 20.0f);
