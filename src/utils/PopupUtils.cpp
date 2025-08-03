// PopupUtils.cpp
#include "PopupUtils.h"
#include <Windows.h>
#include "fonts.h" // For g_unicodeFont

void ShowStyledModalPopup(
    const char *id,
    const char *title,
    const char *message,
    const char *linkLabel,
    const char *linkUrl,
    std::function<void()> onClose)
{
    // Open once
    static bool opened = false;
    if (!opened)
    {
        ImGui::PushFont(g_unicodeFont);
        ImGui::OpenPopup(id);
        ImGui::PopFont();
        opened = true;
    }

    // Center and size
    ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_Always);
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // Colors
    ImVec4 titleBgColor = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    ImVec4 bodyBgColor = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    ImVec4 linkColor = ImVec4(0.3f, 0.6f, 1.0f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_PopupBg, bodyBgColor);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, titleBgColor);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, titleBgColor);
    ImGui::PushFont(g_unicodeFont);

    if (ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (title)
            ImGui::PushFont(g_unicodeFont);
            ImGui::Text("%s", title);
            ImGui::PopFont();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        if (message)
            ImGui::TextWrapped("%s", message);

        if (linkLabel && linkUrl)
        {
            ImGui::Spacing();
            ImGui::TextColored(linkColor, "%s", linkLabel);

            if (ImGui::IsItemHovered())
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();
                ImGui::GetWindowDrawList()->AddLine(min, ImVec2(max.x, max.y), ImGui::GetColorU32(linkColor), 1.0f);
                if (ImGui::IsMouseClicked(0))
                    ShellExecuteA(NULL, "open", linkUrl, NULL, NULL, SW_SHOWNORMAL);
            }
        }

        ImGui::Spacing();
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
            opened = false;
            if (onClose)
                onClose();
        }

        ImGui::EndPopup();
    }

    ImGui::PopFont();
    ImGui::PopStyleColor(3);
}
