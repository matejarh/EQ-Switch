#pragma once
#include <string>

class EqualizerAPOManager {
public:
    static EqualizerAPOManager& getInstance();

    void checkSection();

    // Attempts to detect the installation path
    bool detectInstallation();

    // Returns the config directory (e.g., C:\Program Files\EqualizerAPO\config)
    std::string getConfigDir() const;

    // Returns the path to Editor.exe
    std::string getEditorPath() const;

private:
    EqualizerAPOManager() = default;
    bool isValidInstall(const std::string& basePath);

    std::string installPath;
    std::string configPath;
    std::string editorPath;
};
