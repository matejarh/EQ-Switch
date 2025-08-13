#include "imgui.h"
#include "EQSwitchWindow.h"
#include "utils/AppLauncher.h"
#include "utils/ChannelLabels.h"
#include "gui/_partials/VUMeters.h"
#include "utils/fonts.h"
#include "utils/SystemUtils.h"
#include "utils/EqualizerAPOManager.h"
#include "gui/_partials/ProfilesSelector.h"
#include "utils/PopupUtils.h"
#include <vector>
#include <filesystem>
#include <windows.h>
#include "gui/_partials/Footer.h"
#include "gui/_partials/VUMetersSection.h"
/* #include <d3d11.h> */

void ShowEQSwitchWindow(ProfileManager &profileManager,
                        VUBuffer &vuBuffer,
                        bool *p_exit, float main_scale,
                        EqualizerAPOManager &apoManager,
                        FrequencyVUMeter &frequencyVumeter,
                        AudioCapture &audioCapture, int sampleRate)
{
    static bool showMissingAPOPopup = false;
    static bool showMissingProfilesDirPopup = false;

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
    // Check if profiles directory is accessible
    if (!IsProfilesFolderAccessible(apoManager.getProfilesDir()))
    {
        showMissingProfilesDirPopup = true;
    }

    static std::vector<Profile> profiles = profileManager.loadProfiles();
    static std::string currentProfile = profileManager.getCurrentProfile();
    static int selectedProfile = -1;
    static bool initialized = false;

    // Get the band levels vector from the FrequencyVUMeter inside AudioCapture
    const std::vector<float> &bandLevels = audioCapture.getFrequencyVUMeter().getBandLevels();

    // Also prepare the frequency bands ranges (if your FrequencyVUMeter provides them)
    std::vector<std::pair<float, float>> bandRanges = audioCapture.getFrequencyVUMeter().getBandRanges(sampleRate);

    VuMeters vumeters;

    if (!initialized)
    {
        selectedProfile = findCurrentProfileIndex(profiles, currentProfile);
        initialized = true;
    }

    if (showMissingAPOPopup)
    {
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

    if (showMissingProfilesDirPopup)
    {
        ShowStyledModalPopup(
            "⚠ MissingProfilesDir",
            "Missing Profiles Directory",
            "The profiles directory is not accessible.\n",
            false,
            false,
            []()
            {
                PostQuitMessage(0); // Exit on close
            });
        // showMissingProfilesDirPopup = false; // Trigger only once
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
    ImGui::BeginGroup();

    ImGui::PushFont(g_unicodeFont);
    ImGui::BeginGroup(); // Text block
    ImGui::TextWrapped(
        "EQ Switch is a utility for managing and activating Equalizer APO profiles.\n\n"
        "• Quickly preview and apply profiles from a list.\n"
        "• Especially useful for converting stereo output to 5.1 speaker setups.\n"
        "• Automatically copies selected profile to Equalizer APO's config.txt.\n"
        "• Double-click a profile to apply it instantly.\n"
        "• Use the VU meters to visualize per-channel audio levels.\n\n"
        "Make sure Equalizer APO is installed and your profiles are placed in the 'eq-presets' folder.");
    ImGui::EndGroup();
    ImGui::PopFont();
    ImGui::SameLine();

    ImGui::EndGroup();
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Current Active Profile: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.325f, 0.557f, 0.796f, 1.0f), "%s", currentProfile.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginGroup();
    VUMetersSection(vuBuffer, main_scale);

    ImGui::SameLine();
    vumeters.DrawFrequencyVUMeter(bandLevels, bandRanges, main_scale, frequencyVumeter, true);
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Profile selection section
    if (IsProfilesFolderAccessible(apoManager.getProfilesDir()))
        ProfilesSection(profileManager, currentProfile, selectedProfile, p_exit, apoManager);

    ImGui::Spacing();

    Footer(apoManager);

    ImGui::End();
}