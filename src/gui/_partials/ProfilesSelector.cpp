

#include <vector>

#include "config/ProfileManager.h"
#include "gui/_partials/ProfilesSelector.h"
#include "gui/EQSwitchWindow.h"
#include "utils/fonts.h"
#include "utils/AppLauncher.h"

int findCurrentProfileIndex(const std::vector<Profile> &profiles, const std::string &currentProfile)
{
    for (int i = 0; i < profiles.size(); ++i)
    {
        if (profiles[i].label == currentProfile)
            return i;
    }
    return -1;
}

void ProfilesSection(ProfileManager &profileManager, 
                     std::string &currentProfileOut, 
                     int &selectedProfileOut, 
                     bool *p_exit, 
                     EqualizerAPOManager &apoManager)
{
    // --- Status message for profile apply ---
    static std::string statusMessage;
    static float statusMessageTimer = 0.0f; // in seconds
    static float statusAlpha = 0.0f;
    static const float fadeDuration = 0.5f; // seconds for fade-in/out

    static std::vector<Profile> profiles = profileManager.loadProfiles();
    static std::string currentProfile = profileManager.getCurrentProfile();
    static int selectedProfile = -1;
    static bool initialized = false;

    if (!initialized)
    {
        selectedProfile = findCurrentProfileIndex(profiles, currentProfile);
        initialized = true;
    }

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

            if (ImGui::Selectable(label.c_str(), isSelected))
            {
                selectedProfile = static_cast<int>(i);
                shouldScrollToSelected = false;
            }

            // Tooltip on hover
            if (ImGui::IsItemHovered())
            {
                std::string tooltip = "Doubleclick to apply \"" + profile.label + "\"";
                ImGui::SetTooltip("%s", tooltip.c_str());
            }

            // Apply profile on double-click
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                profileManager.applyProfile(profile);
                currentProfile = profile.label;
                initialized = false;

                // Set feedback message
                statusMessage = "✔ Profile \"" + profile.label + "\" applied.";
                statusMessageTimer = 5.0f; // show message for 3 seconds
            }

            // Scroll if needed
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

    ImGui::BeginGroup();

    if (ImGui::Button("Apply"))
    {
        if (selectedProfile >= 0 && selectedProfile < (int)profiles.size())
        {
            const Profile &profile = profiles[selectedProfile];
            profileManager.applyProfile(profile);
            currentProfile = profile.label; // Optional: update display
            initialized = false;            // Resync selection next frame

            // Set feedback message
            statusMessage = "✔ Profile \"" + profile.label + "\" applied.";
            statusMessageTimer = 5.0f; // show message for 5 seconds
            statusAlpha = 0.0f;        // Start fade-in
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Open Editor"))
    {
        launchEditor(apoManager.getEditorPath(), apoManager.getInstallDir());
    }

    ImGui::SameLine();
    if (ImGui::Button("Exit"))
    {
        if (p_exit)
            *p_exit = true;
    }

    float contentWidth = ImGui::GetContentRegionAvail().x - 0.0f;

    // Show status message (if any)
    if (!statusMessage.empty() && statusMessageTimer > 0.0f)
    {
        float deltaTime = ImGui::GetIO().DeltaTime;

        // Fade in
        if (statusAlpha < 1.0f && statusMessageTimer > 2.5f)
        {
            statusAlpha += deltaTime / fadeDuration;
            if (statusAlpha > 1.0f)
                statusAlpha = 1.0f;
        }

        // Fade out
        if (statusMessageTimer < 1.0f)
        {
            statusAlpha -= deltaTime / fadeDuration;
            if (statusAlpha < 0.0f)
                statusAlpha = 0.0f;
        }

        statusMessageTimer -= deltaTime;
        ImGui::PushFont(g_unicodeFont);
        ImVec4 textColor = ImVec4(0.2f, 0.8f, 0.2f, statusAlpha); // Green with alpha
        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        ImGui::SameLine(contentWidth - ImGui::CalcTextSize(statusMessage.c_str()).x);
        ImGui::TextWrapped("%s", statusMessage.c_str());
        ImGui::PopStyleColor();
        ImGui::PopFont();
    }

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Output updated values
    currentProfileOut = currentProfile;
    selectedProfileOut = selectedProfile;
    // statusMessageOut = statusMessage;
    // statusMessageTimerOut = statusMessageTimer;
}