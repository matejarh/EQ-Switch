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
#include <Footer.h>
#include <VUMetersSection.h>
/* #include <d3d11.h> */


void ShowEQSwitchWindow(ProfileManager &profileManager,
                        VUBuffer &vuBuffer,
                        bool *p_exit, float main_scale,
                        EqualizerAPOManager &apoManager/* ,
                        ID3D11ShaderResourceView* iconTexture */)
{
    static bool showMissingAPOPopup = false;

/*     // Load icon texture from resource
    static ID3D11ShaderResourceView* myIconTexture = nullptr; */

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

/*     if (iconTexture)
    {
        ImGui::Image((void *)iconTexture, ImVec2(128, 128)); // Adjust size as needed
    }
     */

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

    // VU Meters section
    VUMetersSection(vuBuffer, main_scale);

    ImGui::Spacing();

    // Profile selection section
    ProfilesSection(profileManager, currentProfile, selectedProfile, p_exit, apoManager);

    ImGui::Spacing();

    Footer(apoManager);

    ImGui::End();
}