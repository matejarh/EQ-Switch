#pragma once
#include <string>

/**
 * @brief Manages the Equalizer APO installation and configuration.
 * 
 * This class provides methods to detect the installation of Equalizer APO,
 * check the accessibility of the profiles folder, and verify the availability of the Editor.
 * It also provides a singleton instance for easy access throughout the application.
 * 
 */
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

    std::string getProfilesDir() const;
    std::string getInstallDir() const;

private:
    EqualizerAPOManager() = default;
    bool isValidInstall(const std::string& basePath);

    std::string installPath;
    std::string configPath;
    std::string editorPath;
    std::string profilesDir;
};
