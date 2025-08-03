#include "EqualizerAPOManager.h"
#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

EqualizerAPOManager& EqualizerAPOManager::getInstance() {
    static EqualizerAPOManager instance;
    return instance;
}

bool EqualizerAPOManager::detectInstallation() {
    std::vector<std::string> possiblePaths = {
        "C:\\Program Files\\EqualizerAPO",
        "C:\\Program Files (x86)\\EqualizerAPO"
    };

    for (const auto& path : possiblePaths) {
        if (isValidInstall(path)) {
            installPath = path;
            configPath = installPath + "\\config";
            editorPath = installPath + "\\Editor.exe";
            return true;
        }
    }
    return false;
}

bool EqualizerAPOManager::isValidInstall(const std::string& basePath) {
    return fs::exists(basePath + "\\config\\config.txt") &&
           fs::exists(basePath + "\\Editor.exe");
}

std::string EqualizerAPOManager::getConfigDir() const {
    return configPath;
}

std::string EqualizerAPOManager::getEditorPath() const {
    return editorPath;
}
