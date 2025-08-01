#include "VUBuffer.h"
#include "ChannelLabels.h"
#include <imgui.h>
#include "fonts.h"
#include "VUMeters.h"

void VuMeters::VuMeterProgresBar(VUBuffer &vuBuffer, float main_scale)
{
    const int barHeight = 20 * main_scale;
    const int barWidth = 300 * main_scale;

    for (int ch = 0; ch < vuBuffer.getChannelCount(); ++ch)
    {
        float vu = vuBuffer.get(ch);
        std::string label = channelLabels[ch];
        while (label.length() < 3)
            label += ' ';

        ImVec4 color;

        if (vu < 0.6f)
            color = ImVec4(0.2f, 0.9f, 0.2f, 1.0f); // Green
        else if (vu < 0.9f)
            color = ImVec4(0.9f, 0.7f, 0.1f, 1.0f); // Yellow
        else
            color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f); // Red

        // Draw progress bar (without text inside)
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
        ImGui::ProgressBar(vu, ImVec2(barWidth, barHeight), "");
        ImGui::PopStyleColor();

        // Channel label and percentage on same line, in small font
        ImGui::SameLine();
        ImGui::PushFont(g_SmallFontMono);
        ImGui::Text("%s %3.0f%%", label.c_str(), vu * 100.0f);
        ImGui::PopFont();
    }
}

void VuMeters::LedVuMeterHorizontal(VUBuffer &vuBuffer, float main_scale)
{
    const int numLeds = 20;
    const ImVec2 ledSize(16, 24);  // Width x Height of each LED
    const float ledSpacing = 6.0f; // Horizontal space between LEDs
    const float vuWidth = numLeds * (ledSize.x + ledSpacing) - ledSpacing;
    const int numChannels = vuBuffer.getChannelCount();

    ImGui::BeginGroup();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float vu = vuBuffer.get(ch);
        int activeLeds = static_cast<int>(vu * numLeds + 0.5f);

        std::string label = channelLabels[ch];
        while (label.length() < 3)
            label += ' ';

        ImGui::PushID(ch); // Unique ID per channel
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList *drawList = ImGui::GetWindowDrawList();

        // Draw LED row
        for (int i = 0; i < numLeds; ++i)
        {
            float x = pos.x + i * (ledSize.x + ledSpacing);
            ImVec2 ledPosMin = ImVec2(x, pos.y);
            ImVec2 ledPosMax = ImVec2(x + ledSize.x, pos.y + ledSize.y);

            if (i < activeLeds)
            {
                ImU32 color =
                    (i >= numLeds - 2) ? IM_COL32(255, 0, 0, 255) : // last LED red
                        (i >= 12) ? IM_COL32(255, 255, 0, 255)
                                  :               // mid LEDs yellow
                        IM_COL32(0, 255, 0, 255); // low LEDs green

                drawList->AddRectFilled(ledPosMin, ledPosMax, color, 2.0f);
            }
            else
            {
                drawList->AddRect(ledPosMin, ledPosMax, IM_COL32(60, 60, 60, 255), 2.0f);
            }
        }

        // Reserve space for the LED bar
        ImGui::InvisibleButton("##LED_ROW", ImVec2(vuWidth, ledSize.y));

        // Draw channel label and percentage
        ImGui::SameLine();
        ImGui::PushFont(g_SmallFontMono);
        ImGui::Text("%s %3.0f%%", label.c_str(), vu * 100.0f);
        ImGui::PopFont();

        ImGui::PopID();

        // Next row
    }

    ImGui::EndGroup();
}

void VuMeters::LedVuMeter(VUBuffer &vuBuffer, float main_scale)
{
    // VU meters section
    ImGui::BeginGroup();

    const int numLeds = 8;
    const ImVec2 ledSize(30, 20);                                           // LED width x height
    const float ledSpacing = 6.0f;                                          // Gap between LEDs
    const float vuHeight = numLeds * (ledSize.y + ledSpacing) - ledSpacing; // Total height
    const int numChannels = vuBuffer.getChannelCount();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float vu = vuBuffer.get(ch);
        int activeLeds = static_cast<int>(vu * numLeds + 0.5f);

        // Begin full column
        ImGui::BeginGroup();

        ImGui::PushID(ch); // Unique ID for each channel group

        // Calculate position for LEDs
        ImVec2 groupPos = ImGui::GetCursorScreenPos();
        float columnWidth = ledSize.x + ledSpacing; // Width of each column including spacing
        float columnXCenter = groupPos.x + columnWidth * 0.5f;

        // Determine label
        const char *label = (ch < 6) ? channelLabels[ch] : "Ch?";
        ImVec2 labelSize = ImGui::CalcTextSize(label);

        // Position label centered above column
        ImVec2 labelPos(columnXCenter - labelSize.x * 0.5f, groupPos.y);

        // Draw label manually (so it's pixel-accurate)
        ImGui::GetWindowDrawList()->AddText(g_SmallFont, 20.0f, labelPos, IM_COL32(160, 160, 160, 255), label);

        // Reserve space so LED drawing starts below label
        ImGui::Dummy(ImVec2(columnWidth, labelSize.y + 1.0f)); // add vertical spacing

        // --- LED stack with fixed size container ---
        ImVec2 ledStackPos = ImGui::GetCursorScreenPos();
        ImVec2 ledStackSize = ImVec2(ledSize.x, vuHeight);

        ImGui::InvisibleButton("##vu_bg", ledStackSize); // Reserve space
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetItemRectMin();

        for (int i = 0; i < numLeds; ++i)
        {
            float y = pos.y + vuHeight - (i + 1) * (ledSize.y + ledSpacing);

            if (i < activeLeds)
            {
                ImU32 color = (i == numLeds - 1) ? IM_COL32(255, 0, 0, 255) : (i >= 5) ? IM_COL32(255, 255, 0, 255)
                                                                                       : IM_COL32(0, 255, 0, 255);

                drawList->AddRectFilled(
                    ImVec2(pos.x, y),
                    ImVec2(pos.x + ledSize.x, y + ledSize.y),
                    color, 2.0f);
            }
            else
            {
                drawList->AddRect(
                    ImVec2(pos.x, y),
                    ImVec2(pos.x + ledSize.x, y + ledSize.y),
                    IM_COL32(60, 60, 60, 255), 2.0f);
            }
        }
        ImGui::PopID(); // End unique ID for channel group
        ImGui::EndGroup();

        if (ch < numChannels - 1)
            ImGui::SameLine();
    }

    ImGui::EndGroup();
}
