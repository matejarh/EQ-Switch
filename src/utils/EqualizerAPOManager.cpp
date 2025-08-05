#include "EqualizerAPOManager.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <shlwapi.h> // For PathFileExistsA
#include <filesystem>
#include <vector>
#include "SystemUtils.h"
#include "fonts.h"
#include "imgui.h"

#pragma comment(lib, "Shlwapi.lib") // Required for PathFileExists

namespace fs = std::filesystem;

EqualizerAPOManager &EqualizerAPOManager::getInstance()
{
    static EqualizerAPOManager instance;
    return instance;
}

void EqualizerAPOManager::checkSection()
{
    // Perform checks
    bool isAPOOk = detectInstallation();
    bool isProfilesOk = IsProfilesFolderAccessible();
    bool isEditorOk = IsLaunchEditorBatAvailable();

    ImGui::PushFont(g_SmallFont);

    // 1. Start horizontal layout
    ImGui::BeginGroup();
    ImGui::Text("Status:");
    ImVec2 textPos = ImGui::GetCursorScreenPos(); // after the Text

    ImGui::EndGroup();
    ImGui::SameLine(); // move to the same line

    // 2. Compute LED size and center alignment
    float ledSize = 14.0f;
    float spacing = 10.0f;
    float ledsHeight = ledSize;
    float textHeight = ImGui::GetFontSize();
    float yOffset = (textHeight - ledsHeight) * 0.5f;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + yOffset);

    // 3. Draw LED group
    ImGui::BeginGroup();
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 ledPos = ImGui::GetCursorScreenPos();

    // Checks
    bool statusChecks[] = {
        detectInstallation(),
        IsProfilesFolderAccessible(),
        IsLaunchEditorBatAvailable()};

    const char *tooltips[] = {
        "Equalizer APO is installed",
        "Profiles folder is accessible",
        "LaunchEditor.bat is available"};

    for (int i = 0; i < 3; ++i)
    {
        ImU32 color = statusChecks[i] ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);
        ImVec2 center = ImVec2(ledPos.x + ledSize * 0.5f, ledPos.y + ledSize * 0.5f);

        drawList->AddCircleFilled(center, ledSize * 0.5f, color, 20);

        // Create invisible button for hover detection
        ImGui::SetCursorScreenPos(ledPos);
        ImGui::InvisibleButton(("##led" + std::to_string(i)).c_str(), ImVec2(ledSize, ledSize));

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", tooltips[i]);

        // Advance to next LED
        ledPos.x += ledSize + spacing;
    }
    ImGui::EndGroup();

    ImGui::PopFont();
}

// Attempts to detect the installation path
// Checks common installation directories for the presence of Equalizer APO files
// Returns true if a valid installation is found, false otherwise
bool EqualizerAPOManager::detectInstallation()
{
    HKEY hKey;
    const char *registryPath = "SOFTWARE\\EqualizerAPO";

    LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, registryPath, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
    if (result == ERROR_SUCCESS)
    {
        char installDir[MAX_PATH] = {};
        char configDir[MAX_PATH] = {};
        DWORD bufLenInstall = sizeof(installDir);
        DWORD bufLenConfig = sizeof(configDir);
        DWORD type = REG_SZ;

        bool okInstall = RegQueryValueExA(hKey, "installPath", nullptr, &type, reinterpret_cast<LPBYTE>(installDir), &bufLenInstall) == ERROR_SUCCESS;
        bool okConfig = RegQueryValueExA(hKey, "configPath", nullptr, &type, reinterpret_cast<LPBYTE>(configDir), &bufLenConfig) == ERROR_SUCCESS;

        RegCloseKey(hKey);

        if (okInstall)
        {
            installPath = std::string(installDir);
            //std::cout << "✅ Registry installPath: " << installPath << "\n";

            editorPath = installPath + "\\Editor.exe";
        }
        else
        {
            std::cerr << "⚠ Could not read 'installPath' from registry\n";
        }

        if (okConfig)
        {
            configPath = std::string(configDir);
            // std::cout << "✅ Registry configPath: " << configPath << "\n";
        }
        else
        {
            std::cerr << "⚠ Could not read 'configPath' from registry\n";
        }

        // Use installPath as base for validation
        if (okInstall && isValidInstall(installPath))
        {
            return true;
        }
    }
    else
    {
        std::cerr << "❌ Registry key not found: " << registryPath << "\n";
    }

    // Fallback
    std::vector<std::string> fallbackPaths = {
        "C:\\Program Files\\EqualizerAPO",
        "C:\\Program Files (x86)\\EqualizerAPO"};

    for (const auto &path : fallbackPaths)
    {
        if (isValidInstall(path))
        {
            installPath = path;
            configPath = installPath + "\\config";
            editorPath = installPath + "\\Editor.exe";
            std::cout << "✅ Equalizer APO detected via fallback at: " << installPath << "\n";
            return true;
        }
    }

    std::cerr << "❌ Equalizer APO not found.\n";
    return false;
}

bool EqualizerAPOManager::isValidInstall(const std::string &path)
{
    std::string config = path + "\\config\\config.txt";
    return PathFileExistsA(config.c_str());
}

std::string EqualizerAPOManager::getConfigDir() const
{
    return configPath;
}

std::string EqualizerAPOManager::getEditorPath() const
{
    return editorPath;
}
