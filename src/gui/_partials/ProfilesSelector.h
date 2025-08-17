// ProfilesSelector.h
#pragma once
#include <string>
#include "utils/EqualizerAPOManager.h"
#include "config/ProfileManager.h"

class ProfilesSelector
{
public:
    enum class Mode
    {
        ProfilesList,
        ProfilesGrid
    };

    ProfilesSelector()
        : mode(Mode::ProfilesGrid) // Default mode
    {
    }

    void setMode(Mode newMode) { mode = newMode; }
    Mode getMode() const { return mode; }

    void List(ProfileManager &profileManager,
                         std::string &currentProfileOut,
                         int &selectedProfileOut,
                         bool *p_exit,
                         EqualizerAPOManager &apoManager, float main_scale);

    void Grid(
                        ProfileManager &profileManager,
                        std::string &currentProfileOut,
                        int &selectedProfileOut,
                        bool *p_exit,
                        EqualizerAPOManager &apoManager, float main_scale);

    void activeProfile(ProfileManager &profileManager, float main_scale);

    void selectionButtons(ProfileManager &profileManager,std::string &currentProfile,
                     int &selectedProfile,
                     bool *p_exit,
                     EqualizerAPOManager &apoManager, float main_scale);

private:
    Mode mode;
};
