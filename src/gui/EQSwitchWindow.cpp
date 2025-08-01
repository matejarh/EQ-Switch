#include "EQSwitchWindow.h"
#include "utils/AppLauncher.h"
#include "utils/ChannelLabels.h"
#include "utils/VUMeters.h"
#include "imgui.h"
#include <vector>
#include <filesystem>
#include <fonts.h>
#include <SystemUtils.h>

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

    ImGui::PushFont(g_SmallFont);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::TextWrapped("EQ Switch lets you preview and activate Equalizer APO profiles.\nUseful for routing stereo to 5.1 and testing spatial setups.");
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Current Active Profile: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.325f, 0.557f, 0.796f, 1.0f), "%s", currentProfile.c_str());
    ImGui::Separator();
    ImGui::Spacing();

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

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Profile selection section
    ImGui::Text("Available Profiles:");
    ImGui::Spacing();
    static bool shouldScrollToSelected = true;
    ImGui::PushFont(g_SmallFont);
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
    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Apply"))
    {
        if (selectedProfile >= 0 && selectedProfile < (int)profiles.size())
        {
            const Profile &profile = profiles[selectedProfile];
            profileManager.applyProfile(profile);
            currentProfile = profile.label; // Optional: update display
            initialized = false;            // Resync selection next frame
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

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::PushFont(g_SmallFont);
    if (!IsEqualizerAPOInstalled())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Equalizer APO is not installed or not accessible.");
        ImGui::Text("Please install Equalizer APO to use this application.");
    }
    /*     else
        {
            ImGui::Text("Equalizer APO is installed and accessible.");
        } */
    if (!IsProfilesFolderAccessible())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Profiles folder is not accessible.");
        ImGui::Text("Please check the permissions of the profiles folder.");
    }
    /*     else
        {
            ImGui::Text("Profiles folder is accessible.");
        } */
    if (!IsLaunchEditorBatAvailable())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Launch Editor batch file is not available.");
        ImGui::Text("Please ensure the launch editor script is present in the application directory.");
    }
    /*     else
        {
            ImGui::Text("Launch Editor batch file is available.");
        } */

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Version: 1.1.0");
    ImGui::SameLine();
    ImGui::Text("Developed by: Grower");
    ImGui::Spacing();

    ImGui::PopFont();

    ImGui::Separator();

    ImGui::End();
}