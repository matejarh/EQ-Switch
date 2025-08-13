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

/**
 * @brief Singleton instance getter for EqualizerAPOManager.
 *
 * @return EqualizerAPOManager&
 */
EqualizerAPOManager &EqualizerAPOManager::getInstance()
{
    static EqualizerAPOManager instance;
    return instance;
}

/**
 * @brief  Checks the status of Equalizer APO installation and profiles folder accessibility.
 * Displays a horizontal row of status LEDs with tooltips for failed checks, and shows
 * actual paths if checks succeed.
 */
/* void EqualizerAPOManager::checkSection()
{
    ImGui::PushFont(g_SmallFont);

    // Perform checks (only once)
    bool isAPOOk = detectInstallation();
    bool isProfilesOk = IsProfilesFolderAccessible(getProfilesDir());
    bool isEditorOk = editorexeIsAvailableAt(editorPath);

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
         installPath},
        {isProfilesOk,
         "✔ Profiles folder is accessible at",
         "✖ Profiles folder is missing or inaccessible.",
         "EQ profiles should be located in the 'eq-presets' folder.",
         EqualizerAPOManager::getProfilesDir()},
        {isEditorOk,
         "✔ Editor is available at",
         "✖ Editor executable was not found.",
         "Optional: add path to your profile editor in settings.",
         editorPath}};

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
} */

/**
 * @brief Detects the installation of Equalizer APO on a 64-bit Windows system.
 *
 * This method attempts to locate the Equalizer APO installation by querying the Windows registry.
 * It checks the 64-bit registry path:
 *   - HKEY_LOCAL_MACHINE\SOFTWARE\EqualizerAPO
 *
 * If the registry key is found and contains valid values for:
 *   - "InstallDir" – the root installation path of Equalizer APO
 *   - "ConfigPath" – the path to the configuration directory
 *
 * then the method caches the following:
 *   - installPath – installation root directory
 *   - configPath – configuration directory (target for `config.txt`)
 *   - editorPath – path to `Editor.exe` inside the installation folder
 *
 * @return true if a valid Equalizer APO installation is detected and cached, false otherwise.
 */
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
            // std::cout << "✅ Registry installPath: " << installPath << "\n";

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

/**
 * @brief Checks whether the given path is a valid Equalizer APO installation.
 *
 * This function validates the installation by checking if the expected configuration
 * file (`config.txt`) exists in the `config` subdirectory of the provided path.
 *
 * @param path The base installation directory to check.
 * @return true if the `config\\config.txt` file exists under the given path, false otherwise.
 */
bool EqualizerAPOManager::isValidInstall(const std::string &path)
{
    std::string config = path + "\\config\\config.txt";
    return PathFileExistsA(config.c_str());
}

/**
 * @brief Returns the configuration directory for Equalizer APO.
 *
 * @return std::string
 */
std::string EqualizerAPOManager::getConfigDir() const
{
    return configPath;
}

/**
 * @brief Returns the path to the Equalizer APO Editor executable.
 *
 * @return std::string
 */
std::string EqualizerAPOManager::getEditorPath() const
{
    return editorPath;
}

/**
 * @brief Returns the installation directory of Equalizer APO.
 *
 * @return std::string
 */
std::string EqualizerAPOManager::getInstallDir() const
{
    return installPath;
}

/**
 * @brief Returns the directory where Equalizer APO profiles are stored.
 *
 * @return std::string
 */
std::string EqualizerAPOManager::getProfilesDir() const
{
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecA(exePath); // Remove exe filename, leave directory

    // Move up one directory
    PathRemoveFileSpecA(exePath);

    // Append eq-presets
    std::string fullPath = std::string(exePath) + "\\eq-presets\\";

    return fullPath;
}
