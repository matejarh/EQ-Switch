#pragma once
#include <string>
#include <vector>

struct Profile
{
    std::string label;
    std::string filename;
    std::string icon;
};

class ProfileManager
{
public:
    ProfileManager(const std::string &profilesDir, const std::string &configTarget);

    std::vector<Profile> loadProfiles() const;
    std::string getCurrentProfile() const;
    std::string getCurrentProfileIcon() const;
    bool applyProfile(const Profile &p);
    std::string ExtractBaseLabel(const std::string &label);
    // bool launchEditor();

private:
    bool copyFile(const std::string &src, const std::string &dest);

    std::string profilesDir;
    std::string configTarget;
};
