#pragma once
#include <string>
#include <vector>

struct Profile
{
    std::string label;
    std::string filename;
};

class ProfileManager
{
public:
    ProfileManager(const std::string &profilesDir, const std::string &configTarget);

    std::vector<Profile> loadProfiles() const;
    std::string getCurrentProfile() const;
    bool applyProfile(const Profile &p);
    bool launchEditor();

private:
    bool copyFile(const std::string &src, const std::string &dest);
    void restartEqualizerAPO();

    std::string profilesDir;
    std::string configTarget;
};
