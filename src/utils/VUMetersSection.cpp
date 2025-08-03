#include "VUMeters.h"
#include <imgui.h>

void VUMetersSection(VUBuffer &vuBuffer, float main_scale)
{
        // VU Meters section
    static VuMeterMode vuMeterMode = VuMeterMode::LedHorizontal; // Default mode
    const char *modes[] = {"Progress Bar", "LED Horizontal", "LED Vertical"};
    int currentMode = static_cast<int>(vuMeterMode);

    // Find max label width for combo
    float maxWidth = 0.0f;
    for (int i = 0; i < IM_ARRAYSIZE(modes); ++i)
    {
        ImVec2 size = ImGui::CalcTextSize(modes[i]);
        if (size.x > maxWidth)
            maxWidth = size.x;
    }
    maxWidth += ImGui::GetStyle().FramePadding.x * 2 + 10.0f; // Add padding

    // Draw label
    ImGui::Spacing();
    ImGui::Text("VU Meters:");
    ImGui::PushFont(g_SmallFont);
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f); // Fine-tune vertical alignment

    // Set fixed combo width
    ImGui::SetNextItemWidth(maxWidth);
    if (ImGui::Combo("##VU Mode", &currentMode, modes, IM_ARRAYSIZE(modes)))
    {
        vuMeterMode = static_cast<VuMeterMode>(currentMode);
    }
    ImGui::PopFont();
    ImGui::Spacing();
    VuMeters vuMeters;

    switch (vuMeterMode)
    {
    case VuMeterMode::ProgressBar:
        vuMeters.VuMeterProgresBar(vuBuffer, main_scale);
        break;
    case VuMeterMode::LedHorizontal:
        vuMeters.LedVuMeterHorizontal(vuBuffer, main_scale);
        break;
    case VuMeterMode::LedVertical:
        vuMeters.LedVuMeter(vuBuffer, main_scale);
        break;
    }
}