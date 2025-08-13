#include <imgui.h>
#include <fonts.h>
#include "utils/EqualizerAPOManager.h"
#include "utils/SystemUtils.h"

/**
 * @brief  Checks the status of Equalizer APO installation and profiles folder accessibility.
 * Displays a horizontal row of status LEDs with tooltips for failed checks, and shows
 * actual paths if checks succeed.
 */
void checkSection(EqualizerAPOManager &apoManager)
{
    ImGui::PushFont(g_SmallFont);

    // Perform checks (only once)
    bool isAPOOk = apoManager.detectInstallation();
    bool isProfilesOk = IsProfilesFolderAccessible(apoManager.getProfilesDir());
    bool isEditorOk = editorexeIsAvailableAt(apoManager.getEditorPath());

    // Display status label
    ImGui::BeginGroup();
    ImGui::Text("Status:");
    ImGui::EndGroup();
    ImGui::SameLine();

    // LED layout settings
    float ledSize = 14.0f;
    float spacing = 10.0f;
    float textHeight = ImGui::GetFontSize();
    float yOffset = (textHeight - ledSize) * 0.5f;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + yOffset);

    // Draw LEDs
    ImGui::BeginGroup();
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 ledPos = ImGui::GetCursorScreenPos();

    struct StatusCheck
    {
        bool result;
        std::string okMessage;
        std::string errorMessage;
        std::string tooltip;
        std::string extraInfo;
    };

    std::vector<StatusCheck> checks = {
        {isAPOOk,
         "✔ Equalizer APO is installed at",
         "✖ Equalizer APO is not installed or not detected.",
         "Equalizer APO must be properly installed and detected.",
         apoManager.getInstallDir()},
        {isProfilesOk,
         "✔ Profiles folder is accessible at",
         "✖ Profiles folder is missing or inaccessible.",
         "EQ profiles should be located in the 'eq-presets' folder.",
         apoManager.getProfilesDir()},
        {isEditorOk,
         "✔ Editor is available at",
         "✖ Editor executable was not found.",
         "Optional: add path to your profile editor in settings.",
         apoManager.getEditorPath()}};

    for (int i = 0; i < checks.size(); ++i)
    {
        const StatusCheck &check = checks[i];
        ImU32 color = check.result ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);
        ImVec2 center = ImVec2(ledPos.x + ledSize * 0.5f, ledPos.y + ledSize * 0.5f);

        // Draw LED circle
        drawList->AddCircleFilled(center, ledSize * 0.5f, color, 20);

        // Hover detection
        ImGui::SetCursorScreenPos(ledPos);
        ImGui::InvisibleButton(("##led" + std::to_string(i)).c_str(), ImVec2(ledSize, ledSize));

        if (ImGui::IsItemHovered())
        {
            ImGui::PushFont(g_unicodeFont);
            if (check.result)
            {
                ImGui::SetTooltip("%s\n%s", check.okMessage.c_str(), check.extraInfo.c_str());
            }
            else
            {
                ImGui::SetTooltip("%s\n%s", check.errorMessage.c_str(), check.tooltip.c_str());
            }
            ImGui::PopFont();
        }

        ledPos.x += ledSize + spacing;
    }

    ImGui::EndGroup();

    ImGui::PopFont();
}