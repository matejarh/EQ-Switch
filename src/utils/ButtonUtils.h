// PopupUtils.h
#pragma once
#include <string>
#include <imgui.h>

bool DrawIconButton(
    int id,
    const char *icon,
    const std::string &label,
    const std::string &tooltip,
    ImFont *iconFont,
    float iconHeight,
    ImFont *labelFont,
    float textHeight,
    float buttonSize,
    int &selectedIndex,
    int index,
    bool highlight = true,
    float main_scale = 1.0f );
