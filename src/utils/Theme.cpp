// src/utils/Theme.cpp
#include "Theme.h"
#include "imgui.h"

void ApplyEQSwitchDarkTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // VSCode-style dark background and accent colors
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.53f, 0.56f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f); // #21252B
    colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.17f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.20f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.19f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.26f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.31f, 0.34f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.31f, 0.34f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.36f, 0.39f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.41f, 0.44f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.21f, 0.55f, 0.80f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.21f, 0.55f, 0.80f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.25f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.19f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.21f, 0.55f, 0.80f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.21f, 0.55f, 0.80f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.60f, 0.85f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.65f, 0.90f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 0.26f, 0.29f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.31f, 0.34f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.26f, 0.29f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.30f, 0.31f, 0.34f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.35f, 0.36f, 0.39f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.19f, 0.22f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.60f, 0.85f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.30f, 0.31f, 0.34f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.19f, 0.22f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.30f, 0.47f, 0.62f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.60f, 0.85f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    // Optionally tweak padding, rounding etc
    style.WindowRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 3.0f;
}

void ApplyVSCodeDarkModernTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.ScrollbarRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 5.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(8, 5);

    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text]                 = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);  // #d0d0d0
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_WindowBg]             = ImVec4(0.10f, 0.09f, 0.10f, 1.00f);  // #191819
    colors[ImGuiCol_ChildBg]              = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);  // #1e1f1e
    colors[ImGuiCol_PopupBg]              = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_Border]               = ImVec4(0.28f, 0.30f, 0.38f, 0.60f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_TitleBg]              = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_MenuBarBg]            = colors[ImGuiCol_ChildBg];
    colors[ImGuiCol_CheckMark]            = ImVec4(0.20f, 0.80f, 0.50f, 1.00f);
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.40f, 0.60f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]               = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_Header]               = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.25f, 0.28f, 0.30f, 1.00f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.22f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_Separator]            = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_Tab]                  = ImVec4(0.18f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_TabHovered]           = ImVec4(0.26f, 0.30f, 0.35f, 1.00f);
    colors[ImGuiCol_TabActive]            = ImVec4(0.22f, 0.25f, 0.28f, 1.00f);
    colors[ImGuiCol_ResizeGrip]           = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_PlotHistogram]        = ImVec4(0.60f, 0.85f, 0.45f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.70f, 0.95f, 0.55f, 1.00f);
}

