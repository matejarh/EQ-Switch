#include <imgui.h>
#include "fonts.h"

void Footer()
{
    // Move to bottom of window
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() - 10.0f);
    
    // Draw footer
    float contentWidth = ImGui::GetContentRegionAvail().x - 0.0f; // Leave some space for padding
    const char *versionText = "v1.1.0";
    const char *authorText = "Developed by: Grower";

    ImGui::Separator();
    ImGui::Spacing();
    ImGui::PushFont(g_SmallFont);
    // Left-aligned version
    ImGui::Text("%s", versionText);

    // Right-aligned author
    ImGui::SameLine(contentWidth - ImGui::CalcTextSize(authorText).x);
    ImGui::Text("%s", authorText);
    ImGui::PopFont();
}