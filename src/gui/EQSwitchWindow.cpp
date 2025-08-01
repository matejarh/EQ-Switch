#include "EQSwitchWindow.h"
#include "utils/AppLauncher.h"
#include "utils/ChannelLabels.h"
#include "imgui.h"
#include <vector>
#include <filesystem>
#include <fonts.h>

int findCurrentProfileIndex(const std::vector<Profile> &profiles, const std::string &currentProfile)
{
    for (int i = 0; i < profiles.size(); ++i)
    {
        if (profiles[i].label == currentProfile)
            return i;
    }
    return -1;
}

void ShowEQSwitchWindow(ProfileManager &profileManager,
                        VUBuffer &vuBuffer,
                        bool *p_exit, float main_scale)
{
    static std::vector<Profile> profiles = profileManager.loadProfiles();
    static std::string currentProfile = profileManager.getCurrentProfile();
    static int selectedProfile = -1;
    static bool initialized = false;

    if (!initialized)
    {
        selectedProfile = findCurrentProfileIndex(profiles, currentProfile);
        initialized = true;
    }

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("EQ Switch", p_exit,
                 ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Current Active Profile: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", currentProfile.c_str());
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("VU Meters:");

    const int barHeight = 20 * main_scale;
    const int barWidth = 300 * main_scale;

    for (int ch = 0; ch < vuBuffer.getChannelCount(); ++ch)
    {
        float vu = vuBuffer.get(ch);
        ImVec4 color;

        if (vu < 0.5f)
            color = ImVec4(0.2f, 0.9f, 0.2f, 1.0f); // Green
        else if (vu < 0.8f)
            color = ImVec4(0.9f, 0.7f, 0.1f, 1.0f); // Yellow
        else
            color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f); // Red

        // Draw progress bar (without text inside)
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
        ImGui::ProgressBar(vu, ImVec2(barWidth, barHeight), "");
        ImGui::PopStyleColor();

        // Channel label and percentage on same line, in small font
        ImGui::SameLine();
        ImGui::PushFont(g_SmallFont);
        ImGui::Text("%s  %.0f%%", channelLabels[ch], vu * 100.0f);
        ImGui::PopFont();
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("Available Profiles:");

    static bool shouldScrollToSelected = true;

    if (ImGui::BeginListBox("##ProfileList", ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 8)))
    {
        for (size_t i = 0; i < profiles.size(); ++i)
        {
            const Profile &profile = profiles[i];
            std::string label = profile.label;
            bool isSelected = (selectedProfile == i);

            if (ImGui::Selectable(profile.label.c_str(), isSelected))
            {
                selectedProfile = static_cast<int>(i);
                shouldScrollToSelected = false;
            }

            // Apply profile on double-click
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) // 0 = left mouse button
            {
                profileManager.applyProfile(profile);
                currentProfile = profile.label;
                initialized = false; // Resync selection next frame
            }

            // Auto-scroll if selected item is not visible
            if (isSelected && shouldScrollToSelected && !ImGui::IsItemVisible())
            {
                ImGui::SetScrollHereY(0.5f);
                shouldScrollToSelected = false;
            }
        }

        ImGui::EndListBox();
    }

    if (ImGui::Button("Apply"))
    {
        if (selectedProfile >= 0 && selectedProfile < (int)profiles.size())
        {
            const Profile &profile = profiles[selectedProfile];
            profileManager.applyProfile(profile);
            currentProfile = profile.label; // Optional: update display
            initialized = false; // Resync selection next frame
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Open Editor"))
    {
        launchEditor();
    }
    ImGui::SameLine();
    if (ImGui::Button("Exit"))
    {
        if (p_exit)
            *p_exit = true;
    }
    /* if (ImGui::Button("Launch Editor", ImVec2(600 * main_scale, 22 * main_scale))) */

    /* for (size_t i = 0; i < profiles.size(); ++i)
    {
        const Profile &profile = profiles[i];
        std::string label = profile.label;

        if (ImGui::Selectable(label.c_str(), selectedProfile == i, 0, ImVec2(200, 40)))
        {
            selectedProfile = static_cast<int>(i);
            profileManager.applyProfile(profile);
            currentProfile = profile.label;
        }
    } */

    ImGui::End();
}
