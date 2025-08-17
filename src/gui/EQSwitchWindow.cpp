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
            main_scale,
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
            main_scale,
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

    
    const char* headerText = R"( ________  ______        ______               __   __              __ 
|        \/      \      /      \             |  \ |  \            |  \
| ▓▓▓▓▓▓▓▓  ▓▓▓▓▓▓\    |  ▓▓▓▓▓▓\__   __   __ \▓▓_| ▓▓_    _______| ▓▓____
| ▓▓__   | ▓▓  | ▓▓    | ▓▓___\▓▓  \ |  \ |  \  \   ▓▓ \  /       \ ▓▓    \
| ▓▓  \  | ▓▓  | ▓▓     \▓▓    \| ▓▓ | ▓▓ | ▓▓ ▓▓\▓▓▓▓▓▓ |  ▓▓▓▓▓▓▓ ▓▓▓▓▓▓▓\
| ▓▓▓▓▓  | ▓▓ _| ▓▓     _\▓▓▓▓▓▓\ ▓▓ | ▓▓ | ▓▓ ▓▓ | ▓▓ __| ▓▓     | ▓▓  | ▓▓
| ▓▓_____| ▓▓/ \ ▓▓    |  \__| ▓▓ ▓▓_/ ▓▓_/ ▓▓ ▓▓ | ▓▓|  \ ▓▓_____| ▓▓  | ▓▓
| ▓▓     \\▓▓ ▓▓ ▓▓     \▓▓    ▓▓\▓▓   ▓▓   ▓▓ ▓▓  \▓▓  ▓▓\▓▓     \ ▓▓  | ▓▓
 \▓▓▓▓▓▓▓▓ \▓▓▓▓▓▓\      \▓▓▓▓▓▓  \▓▓▓▓▓\▓▓▓▓ \▓▓   \▓▓▓▓  \▓▓▓▓▓▓▓\▓▓   \▓▓
               \▓▓▓)";

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::BeginGroup();

    ImGui::PushFont(g_SmallFontMono);
    ImVec2 textSize = ImGui::CalcTextSize(headerText);
    float windowWidth = ImGui::GetWindowSize().x;
    ImGui::SetCursorPosX((windowWidth - textSize.x) * 0.5f);
    ImGui::TextUnformatted(headerText);
    ImGui::PopFont();
    ImGui::Spacing();

    ImGui::EndGroup();
    ImGui::PopStyleColor(); 
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
    {
        static ProfilesSelector profilesSelector;

        profilesSelector.selectionButtons(profileManager, currentProfile,
                                          selectedProfile,
                                          p_exit,
                                          apoManager, main_scale);
    }
    Footer(apoManager);

    ImGui::End();
}