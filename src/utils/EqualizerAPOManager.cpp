#include "EqualizerAPOManager.h"
#include <windows.h>
#include <shlwapi.h> // For PathFileExistsA
#include <filesystem>
#include <vector>

#pragma comment(lib, "Shlwapi.lib") // Required for PathFileExists

namespace fs = std::filesystem;

EqualizerAPOManager &EqualizerAPOManager::getInstance()
{
    static EqualizerAPOManager instance;
    return instance;
}

// Attempts to detect the installation path
// Checks common installation directories for the presence of Equalizer APO files
// Returns true if a valid installation is found, false otherwise
bool EqualizerAPOManager::detectInstallation()
{
    // Try registry first
    HKEY hKey;
    const char *registryPaths[] = {
        "SOFTWARE\\EqualizerAPO",             // 64-bit
        "SOFTWARE\\WOW6432Node\\EqualizerAPO" // 32-bit fallback
    };

    for (const auto &regPath : registryPaths)
    {
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            char installDir[MAX_PATH];
            DWORD bufLen = sizeof(installDir);
            DWORD type = REG_SZ;

            if (RegQueryValueExA(hKey, "InstallDir", nullptr, &type, reinterpret_cast<LPBYTE>(installDir), &bufLen) == ERROR_SUCCESS)
            {
                RegCloseKey(hKey);

                std::string path = std::string(installDir);
                if (isValidInstall(path))
                {
                    installPath = path;
                    configPath = installPath + "\\config";
                    editorPath = installPath + "\\Editor.exe";
                    return true;
                }
            }
            else
            {
                RegCloseKey(hKey);
            }
        }
    }

    // Fallback to common directories
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
            return true;
        }
    }

    return false;
}

bool EqualizerAPOManager::isValidInstall(const std::string& path) {
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
