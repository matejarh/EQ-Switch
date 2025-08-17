// PopupUtils.h
#pragma once
#include <string>
#include <functional>
#include <imgui.h>

// Call this each frame in your main loop
void ShowStyledModalPopup(
    const char* id,                       // Unique popup ID
    const char* title,                   // Title with optional emoji
    const char* message,                 // Main body text
    const char* linkLabel = nullptr,     // Optional clickable label
    const char* linkUrl = nullptr,       // Optional URL
    const float main_scale = 1.0f,
    std::function<void()> onClose = {}   // Optional close callback
);
