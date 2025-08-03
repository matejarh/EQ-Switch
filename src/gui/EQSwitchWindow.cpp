#include "imgui.h"
#include "EQSwitchWindow.h"
#include "utils/AppLauncher.h"
#include "utils/ChannelLabels.h"
#include "utils/VUMeters.h"
#include "utils/fonts.h"
#include "utils/SystemUtils.h"
#include "utils/EqualizerAPOManager.h"
#include "utils/ProfilesSelector.h"
#include <PopupUtils.h>
#include <vector>
#include <filesystem>
#include <windows.h>

/* int findCurrentProfileIndex(const std::vector<Profile> &profiles, const std::string &currentProfile)
{
    for (int i = 0; i < profiles.size(); ++i)
    {
        if (profiles[i].label == currentProfile)
            return i;
    }
    return -1;
} */

void ShowEQSwitchWindow(ProfileManager &profileManager,
                        VUBuffer &vuBuffer,
                        bool *p_exit, float main_scale)
{
    auto &apoManager = EqualizerAPOManager::getInstance();
    static bool showMissingAPOPopup = false;

    // Check if APO is detected and trigger popup
    static bool apoChecked = false;
    if (!apoChecked)
    {
        apoChecked = true;
        if (!apoManager.detectInstallation())
        {
            showMissingAPOPopup = true;
        }
    }

    static std::vector<Profile> profiles = profileManager.loadProfiles();
    static std::string currentProfile = profileManager.getCurrentProfile();
    static int selectedProfile = -1;
    static bool initialized = false;

    if (!initialized)
    {
        selectedProfile = findCurrentProfileIndex(profiles, currentProfile);
        initialized = true;
    }

    if (showMissingAPOPopup)
    {
        /* ImGui::PushFont(g_unicodeFont); // <== your loaded unicode-compatible font
        ImGui::OpenPopup("⚠ Equalizer APO Not Found");
        ImGui::PopFont(); */
        ShowStyledModalPopup(
            "⚠ MissingAPO",
            "Equalizer APO Not Found",
            "Equalizer APO was not detected on this system.",
            "Download Equalizer APO",
            "https://sourceforge.net/projects/equalizerapo/",
            []()
            {
                PostQuitMessage(0); // Exit on close
            });
        // showMissingAPOPopup = false; // Trigger only once
    }

    // Centered modal
    ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_Always);
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // Custom colors
    ImVec4 titleBgColor = ImVec4(0.15f, 0.15f, 0.15f, 1.0f); // Dark
    ImVec4 bodyBgColor = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);  // Slightly lighter

    ImGui::PushStyleColor(ImGuiCol_PopupBg, bodyBgColor);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, titleBgColor);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, titleBgColor);
    ImGui::PushFont(g_unicodeFont); // <== your loaded unicode-compatible font
    if (ImGui::BeginPopupModal("⚠ Equalizer APO Not Found", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped("Equalizer APO was not detected on this system.");
        ImGui::Spacing();

        // Draw a link-style label
        ImVec4 linkColor = ImVec4(0.3f, 0.6f, 1.0f, 1.0f); // bluish
        ImGui::TextColored(linkColor, "Download Equalizer APO");

        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(min.x, max.y),
                ImVec2(max.x, max.y),
                ImGui::GetColorU32(linkColor),
                1.0f);
        }

        // If clicked, open URL
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
        {
            ShellExecuteA(NULL, "open", "https://sourceforge.net/projects/equalizerapo/", NULL, NULL, SW_SHOWNORMAL);
        }

        ImGui::Spacing();
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
            showMissingAPOPopup = false;
            PostQuitMessage(0); // Exit the app
        }

        ImGui::EndPopup();
    }
    ImGui::PopFont(); // Pop unicode font

    ImGui::PopStyleColor(3); // Restore colors

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

    // Profiles section
    ProfilesSection(profileManager, currentProfile, selectedProfile, p_exit);

/*     ImGui::Spacing();
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
    ImGui::Spacing(); */
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