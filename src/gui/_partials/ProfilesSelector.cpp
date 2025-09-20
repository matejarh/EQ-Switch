// ProfileSelector.cpp

#include <vector>
#include <algorithm>
#include "imgui.h"

#include "config/ProfileManager.h"
#include "gui/_partials/ProfilesSelector.h"
#include "gui/EQSwitchWindow.h"
#include "utils/fonts.h"
#include "utils/AppLauncher.h"
#include <ButtonUtils.h>
#include <PopupUtils.h>
#include <imgui_internal.h>

// ---------- Shared State for Feedback ----------
namespace
{
    std::string statusMessage;
    float statusMessageTimer = 0.0f;
    float statusAlpha = 0.0f;
    constexpr float fadeDuration = 0.5f;
    static bool showAboutPopup = false;
}

// ---------- Helpers ----------
int findCurrentProfileIndex(const std::vector<Profile> &profiles, const std::string &currentProfile)
{
    for (int i = 0; i < (int)profiles.size(); ++i)
        if (profiles[i].label == currentProfile)
            return i;
    return -1;
}

void applyProfileWithFeedback(ProfileManager &manager, const Profile &profile, std::string &currentProfile, std::string &currentProfileIcon)
{
    manager.applyProfile(profile);
    currentProfile = profile.label;
    currentProfileIcon = profile.icon;
    statusMessage = "✔ Profile \"" + profile.label + "\" applied.";
    statusMessageTimer = 5.0f;
    statusAlpha = 0.0f;
}

void renderStatusMessage(const float buttonSize)
{
    if (statusMessage.empty() || statusMessageTimer <= 0.0f)
        return;

    float deltaTime = ImGui::GetIO().DeltaTime;

    // Fade in/out
    if (statusAlpha < 1.0f && statusMessageTimer > 2.5f)
        statusAlpha = (std::min)(1.0f, statusAlpha + deltaTime / fadeDuration);
    if (statusMessageTimer < 1.0f)
        statusAlpha = (std::max)(0.0f, statusAlpha - deltaTime / fadeDuration);

    statusMessageTimer -= deltaTime;

    ImGui::PushFont(g_unicodeFont);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, statusAlpha));
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(statusMessage.c_str()).x - buttonSize);
    ImGui::TextWrapped("%s", statusMessage.c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

void renderFooterButtons(ProfileManager &profileManager,
                         EqualizerAPOManager &apoManager,
                         int &selectedProfile,
                         const std::vector<Profile> &profiles,
                         std::string &currentProfile,
                         std::string &currentProfileIcon,
                         bool *p_exit, float main_scale)
{
    static const std::string applyIcon = u8"\uf04b";
    static const std::string openEditorIcon = u8"\uf044";
    static const std::string exitIcon = u8"\uf00d";
    static const std::string aboutIcon = u8"\uf059"; // question
    const ImVec2 &buttonSize = ImVec2(48.0f * main_scale, 32.0f * main_scale);

    if (showAboutPopup)
    {
        ShowStyledModalPopup(
            "⚠ About EQ Switch",
            "EQ Switch - About",
            "EQ Switch is a utility for managing and activating Equalizer APO profiles.\n\n"
            "• Quickly preview and apply profiles from a list.\n"
            "• Especially useful for converting stereo output to 5.1 speaker setups.\n"
            "• Automatically copies selected profile to Equalizer APO's config.txt.\n"
            "• Double-click a profile to apply it instantly.\n"
            "• Use the VU meters to visualize per-channel audio levels.\n\n"
            "Make sure Equalizer APO is installed and your profiles are placed in the 'eq-presets' folder.",
            false,
            false,
            main_scale,
            []()
            {
                showAboutPopup = false; // Exit on close
            });
        // showMissingAPOPopup = false; // Trigger only once
    }

    ImGui::PushFont(g_fontAwesome);
    if (ImGui::Button(applyIcon.c_str(), buttonSize))
    {
        if (selectedProfile >= 0 && selectedProfile < (int)profiles.size())
        {
            applyProfileWithFeedback(profileManager, profiles[selectedProfile], currentProfile, currentProfileIcon);
        }
    }
    ImGui::PopFont();
    ImGui::PushFont(g_unicodeFont);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Apply \"%s\"", profiles[selectedProfile].label.c_str());
    ImGui::PopFont();

    ImGui::SameLine();

    ImGui::PushFont(g_fontAwesome);
    if (ImGui::Button(openEditorIcon.c_str(), buttonSize))
        launchEditor(apoManager.getEditorPath(), apoManager.getInstallDir());
    ImGui::PopFont();
    ImGui::PushFont(g_unicodeFont);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Open Equalizer APO editor");
    ImGui::PopFont();

    ImGui::SameLine();

    ImGui::PushFont(g_fontAwesome);
    if (ImGui::Button(exitIcon.c_str(), buttonSize) && p_exit)
        *p_exit = true;
    ImGui::PopFont();
    ImGui::PushFont(g_unicodeFont);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Exit application");
    ImGui::PopFont();

    // --- Right-aligned button ---
    float footerWidth = ImGui::GetContentRegionAvail().x; // available width after above items
    float buttonX = ImGui::GetCursorPosX() + footerWidth - buttonSize.x;

    ImGui::SameLine();
    ImGui::SetCursorPosX(buttonX);

    ImGui::PushFont(g_fontAwesome);
    if (ImGui::Button(aboutIcon.c_str(), buttonSize))
    {
        showAboutPopup = true;
    }
    ImGui::PopFont();
    ImGui::PushFont(g_unicodeFont);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("About EQ Switch");
    ImGui::PopFont();

    renderStatusMessage(buttonSize.x);

    ImGui::Spacing();
    ImGui::Separator();
}

// ---------- UI Sections ----------
void ProfilesSelector::activeProfile(ProfileManager &profileManager, float main_scale)
{
    std::string currentProfile = profileManager.getCurrentProfile();
    std::string currentProfileIcon = profileManager.getCurrentProfileIcon();
    ImGui::Spacing();
    ImGui::BeginGroup();

    // --- Measure profile text size with big font ---
    ImGui::PushFont(g_BigFont);
    ImVec2 profileSize = ImGui::CalcTextSize(currentProfile.c_str());
    ImGui::PopFont();

    // --- Measure icon size at its default font ---
    ImGui::PushFont(g_fontAwesome);
    ImVec2 iconSize = ImGui::CalcTextSize(currentProfileIcon.c_str());
    ImGui::PopFont();

    // --- Compute combined width ---
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float totalCenterWidth = iconSize.x + spacing + profileSize.x;
    float windowWidth = ImGui::GetWindowSize().x;
    float startX = (windowWidth - totalCenterWidth) * 0.5f;

    float baseY = ImGui::GetCursorPosY();

    // --- Scale icon to match profile text height ---
    float scale = profileSize.y / iconSize.y;

    

    // Draw icon
    ImGui::SetCursorPosX(startX);
    ImGui::SetCursorPosY(baseY - (2.0f * main_scale));
    ImGui::PushFont(g_fontAwesome);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0)); // optional
    ImGui::SetWindowFontScale(scale);                              // scale the icon vertically
    ImGui::TextColored(ImVec4(0.325f, 0.557f, 0.796f, 1.0f), "%s", currentProfileIcon.c_str());
    ImGui::SetWindowFontScale(1.0f); // reset scale
    ImGui::PopStyleVar();
    ImGui::PopFont();

    // --- Draw profile text ---
    ImGui::SameLine(0.0f, spacing);
    ImGui::SetCursorPosY(baseY);
    ImGui::PushFont(g_BigFont);
    ImGui::TextColored(ImVec4(0.325f, 0.557f, 0.796f, 1.0f), "%s", currentProfile.c_str());
    ImGui::PopFont();

    ImGui::EndGroup();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void ProfilesSelector::selectionButtons(ProfileManager &profileManager,
                                        std::string &currentProfile,
                                        int &selectedProfile,
                                        bool *p_exit,
                                        EqualizerAPOManager &apoManager,
                                        float main_scale)
{
    activeProfile(profileManager, main_scale);

    Mode currentMode = getMode();
    static const char * listIcon = u8"\uf03a";
    static const char * gridIcon = u8"\uf58d";
    static const float buttonSize = 32.0f * main_scale;
    float contentWidth = ImGui::GetContentRegionAvail().x - 0.0f; // Leave some space for padding
    ImVec2 itemSpacing = ImGui::GetStyle().ItemSpacing;

    
    ImGui::BeginGroup();
    const char * labelText = "Available Profiles:";
    // Measure text height
    ImVec2 textSize = ImGui::CalcTextSize(labelText);
    float lineHeight = buttonSize; // height of current font
    float offsetY = (lineHeight - textSize.y) * 0.5f; // center vertically
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
    ImGui::Text("%s", labelText);
    ImGui::EndGroup();
    
    ImGui::PushFont(g_fontAwesome);
    ImGui::SameLine(contentWidth - (buttonSize * 2) - (itemSpacing.x / 2.0f));
    ImGui::PopFont();

    ImGui::BeginGroup();
    ImGui::PushFont(g_fontAwesome);

    // List Mode
    if (currentMode == Mode::ProfilesList)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
    }
    if (ImGui::Button(listIcon, ImVec2(buttonSize, buttonSize)))
        setMode(Mode::ProfilesList);
    if (currentMode == Mode::ProfilesList)
        ImGui::PopStyleColor(2);

    ImGui::PushFont(g_unicodeFont);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("View as List");
    ImGui::PopFont();

    ImGui::SameLine();

    // Grid Mode
    if (currentMode == Mode::ProfilesGrid)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
    }
    if (ImGui::Button(gridIcon, ImVec2(buttonSize, buttonSize)))
        setMode(Mode::ProfilesGrid);
    if (currentMode == Mode::ProfilesGrid)
        ImGui::PopStyleColor(2);
        ImGui::PushFont(g_unicodeFont);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("View As Grid");
        ImGui::PopFont();
        
    ImGui::PopFont();
    ImGui::EndGroup();

    if (currentMode == Mode::ProfilesList)
        List(profileManager, currentProfile, selectedProfile, p_exit, apoManager, main_scale);
    else
        Grid(profileManager, currentProfile, selectedProfile, p_exit, apoManager, main_scale);
}

// ---------- List View ----------
void ProfilesSelector::List(ProfileManager &profileManager,
                            std::string &currentProfileOut,
                            int &selectedProfileOut,
                            bool *p_exit,
                            EqualizerAPOManager &apoManager, float main_scale)
{
    std::vector<Profile> profiles = profileManager.loadProfiles();
    static int selectedProfile = -1; // persist across frames
    static bool initialized = false;
    std::string currentProfile = profileManager.getCurrentProfile();
    std::string currentProfileIcon = profileManager.getCurrentProfileIcon();
    static bool shouldScrollToSelected = false; // <--- NEW

    // Initialize once from currentProfile
    if (!initialized && !profiles.empty())
    {
        selectedProfile = findCurrentProfileIndex(profiles, currentProfile);
        initialized = true;
        shouldScrollToSelected = true; // <--- flag scroll
    }

    ImGui::Spacing();
    ImGui::PushFont(g_SmallFont);

    if (ImGui::BeginListBox("##ProfileList", ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 8)))
    {
        for (size_t i = 0; i < profiles.size(); ++i)
        {
            bool isSelected = (selectedProfile == (int)i);
            if (ImGui::Selectable(profiles[i].label.c_str(), isSelected))
            {
                selectedProfile = (int)i;
                shouldScrollToSelected = false; // user selected manually
            }

            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Doubleclick to apply \"%s\"", profiles[i].label.c_str());
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    applyProfileWithFeedback(profileManager, profiles[i], currentProfile, currentProfileIcon);
                    selectedProfile = (int)i;
                    initialized = false; // re-sync next frame
                }
            }
            // Ensure the selected profile is scrolled into view at start
            if (isSelected && shouldScrollToSelected && !ImGui::IsItemVisible())
            {
                ImGui::SetScrollHereY(0.5f); // center it
                shouldScrollToSelected = false;
            }
        }
        ImGui::EndListBox();
    }
    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    renderFooterButtons(profileManager, apoManager, selectedProfile, profiles, currentProfile, currentProfileIcon, p_exit, main_scale);

    currentProfileOut = currentProfile;
    selectedProfileOut = selectedProfile;
}

// ---------- Grid View ----------
void ProfilesSelector::Grid(ProfileManager &profileManager,
                            std::string &currentProfileOut,
                            int &selectedProfileOut,
                            bool *p_exit,
                            EqualizerAPOManager &apoManager,
                            float main_scale)
{
    std::vector<Profile> profiles = profileManager.loadProfiles();
    std::string currentProfile = profileManager.getCurrentProfile();
    std::string currentProfileIcon = profileManager.getCurrentProfileIcon();

    int selectedProfile = findCurrentProfileIndex(profiles, currentProfile);

    ImGui::Spacing();

    // Layout
    float baseButtonSize = 72.0f;
    float buttonSize = baseButtonSize * main_scale;
    float iconSize = (buttonSize - 26.0f) * 0.6f;
    float textHeight = 14.0f * main_scale;
    float spacing = ImGui::GetStyle().ItemSpacing.x * main_scale;
    float regionWidth = ImGui::GetContentRegionAvail().x;
    int buttonsPerRow = (std::max)(1, (int)((regionWidth + spacing) / (buttonSize + spacing)));

    for (size_t i = 0; i < profiles.size(); ++i)
    {
        bool isSelected = (selectedProfile == (int)i);

        if (isSelected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
        }

        if (DrawIconButton(
                i, profiles[i].icon.c_str(),
                profileManager.ExtractBaseLabel(profiles[i].label),
                "Click to apply \"" + profiles[i].label + "\"",
                g_fontAwesome, iconSize,
                g_LabelFont, textHeight,
                buttonSize, selectedProfile, i, isSelected, main_scale))
        {
            applyProfileWithFeedback(profileManager, profiles[i], currentProfile, currentProfileIcon);
            selectedProfile = (int)i;
        }

        if (isSelected)
            ImGui::PopStyleColor(2);
        if ((i + 1) % buttonsPerRow != 0)
            ImGui::SameLine();
        else
            ImGui::Dummy(ImVec2(0.0f, spacing));
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    renderFooterButtons(profileManager, apoManager, selectedProfile, profiles, currentProfile, currentProfileIcon, p_exit, main_scale);

    currentProfileOut = currentProfile;
    selectedProfileOut = selectedProfile;
}
