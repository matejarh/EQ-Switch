#pragma once
#include <imgui.h>
#include <sstream>
#include <vector>
#include <string>
#include "utils/ButtonUtils.h"
#include "utils/fonts.h"
#include <imgui_internal.h>

// Helper to draw icon + wrapped bottom-aligned label
void DrawButtonIconLabel(
    ImVec2 pos,
    float buttonSize,
    const char *icon,
    ImFont *iconFont,
    float iconHeight,
    const std::string &label,
    ImFont *labelFont,
    float textHeight,
    ImU32 textColor = IM_COL32(255, 255, 255, 255), float main_scale = 1.0f)
{
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    // --- Draw icon ---
    ImVec2 iconSize = iconFont->CalcTextSizeA(iconHeight, FLT_MAX, 0.0f, icon);

    // Horizontal center
    float iconX = pos.x + (buttonSize - iconSize.x) * 0.5f;

    // Vertical center
    float iconY = pos.y + ((buttonSize - 24.0f * main_scale) - iconSize.y) * 0.5f;

    ImGui::GetWindowDrawList()->AddText(
        iconFont,
        iconHeight,
        ImVec2(iconX, iconY),
        textColor,
        icon);

    // --- Draw label ---
    ImGui::PushFont(labelFont);
    // ImGui::PushTextWrapPos(pos.x + buttonSize);

    // --- Prepare wrapped lines for label ---
    float padding = 4.0f * main_scale;
    float wrapWidth = buttonSize - padding; // optional padding
    std::vector<std::string> lines;
    std::istringstream iss(label);
    std::string word;
    while (iss >> word)
    {
        if (!lines.empty() && labelFont->CalcTextSizeA(textHeight, FLT_MAX, 0.0f, (lines.back() + " " + word).c_str()).x < wrapWidth)
            lines.back() += " " + word;
        else
            lines.push_back(word);
    }

    // --- Draw label bottom-aligned ---
    float y = pos.y + buttonSize - lines.size() * textHeight - padding;
    for (auto &line : lines)
    {
        float textWidth = labelFont->CalcTextSizeA(textHeight, FLT_MAX, 0.0f, line.c_str()).x;
        float x = pos.x + (buttonSize - textWidth) * 0.5f; // center
        drawList->AddText(labelFont, textHeight, ImVec2(x, y), textColor, line.c_str());
        y += textHeight;
    }
    ImGui::PopFont();
}

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
    bool highlight, float main_scale)
{
    ImGui::BeginGroup();
    ImGui::PushID(id);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size(buttonSize, buttonSize);

    // Invisible button for hit box
    ImGui::InvisibleButton("##btn", size);
    ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);

    // Optional highlight
    if ((highlight && selectedIndex == index))
    {
        ImVec2 br(pos.x + size.x, pos.y + size.y);
        ImGui::GetWindowDrawList()->AddRectFilled(
            pos, br, (ImGui::GetColorU32(ImGuiCol_ButtonActive)), 6.0f);
        ImGui::GetWindowDrawList()->AddRect(
            pos, br, IM_COL32(255, 255, 255, 255), 6.0f, 0, 2.0f);

        textColor = IM_COL32(255, 255, 255, 255);
    }

    // Optional highlight at hover
    if (ImGui::IsItemHovered() && (!highlight && selectedIndex != index))
        {
        ImVec2 br(pos.x + size.x, pos.y + size.y);
        ImGui::GetWindowDrawList()->AddRectFilled(
            pos, br, (ImGui::GetColorU32(ImGuiCol_ButtonHovered)), 6.0f);
        ImGui::GetWindowDrawList()->AddRect(
            pos, br, IM_COL32(255, 255, 255, 200), 6.0f, 0, 2.0f);

        textColor = IM_COL32(255, 255, 255, 200);
    }

    // Draw icon + label
    DrawButtonIconLabel(pos, buttonSize, icon, iconFont, iconHeight, label, labelFont, textHeight, textColor, main_scale);
    
    // Tooltip
    ImGui::PushFont(g_unicodeFont);
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(tooltip.c_str());
    }
    ImGui::PopFont();
    
    ImGui::PopID();
    ImGui::EndGroup();

    // Detect click on release inside button
    bool clicked = ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
    if (clicked && (!highlight && selectedIndex != index))
        selectedIndex = index;

    return clicked;
}