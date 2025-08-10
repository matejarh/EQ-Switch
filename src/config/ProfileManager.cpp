// ProfileManager.cpp

#include "ProfileManager.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <Windows.h>

namespace fs = std::filesystem;

ProfileManager::ProfileManager(const std::string &profilesDir, const std::string &configTarget)
    : profilesDir(profilesDir), configTarget(configTarget) {}

std::vector<Profile> ProfileManager::loadProfiles() const
{
    std::vector<Profile> profiles;

    if (!fs::exists(profilesDir) || !fs::is_directory(profilesDir))
    {
        std::cerr << "Profiles directory not found: " << profilesDir << "\n";
        return profiles;
    }

    constexpr std::string_view tag = "# EQ Profile: ";

    for (const auto &entry : fs::directory_iterator(profilesDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
        {
            std::ifstream file(entry.path());
            if (!file.is_open())
                continue;

            std::string filename = entry.path().filename().string();
            std::string label;
            std::string line;
            bool foundTag = false;

            while (std::getline(file, line))
            {
                if (line.rfind(tag, 0) == 0) // line starts with tag
                {
                    label = line.substr(tag.size());
                    foundTag = true;
                    break;
                }
            }

            if (foundTag)
            {
                profiles.push_back({label, filename});
            }
            // else skip this file because no tag found
        }
    }
    return profiles;
}

std::string ProfileManager::getCurrentProfile() const
{
    std::ifstream file(configTarget);
    if (!file.is_open())
    {
        return "(Could not read active profile)";
    }

    std::string line;
    constexpr std::string_view tag = "# EQ Profile: ";
    while (std::getline(file, line))
    {
        if (line.rfind(tag, 0) == 0)
        {
            return line.substr(tag.size());
        }
    }
    return "(No profile comment found)";
}

bool ProfileManager::copyFile(const std::string &src, const std::string &dest)
{
    std::ifstream source(src, std::ios::binary);
    std::ofstream destination(dest, std::ios::binary);
    if (!source || !destination)
        return false;

    destination << source.rdbuf();
    return true;
}

bool ProfileManager::applyProfile(const Profile &p)
{
    fs::path srcFile = fs::path(profilesDir) / p.filename;

    std::cout << "Applying profile: " << p.label << "\n"
              << "Source: " << srcFile << "\n"
              << "Target: " << configTarget << "\n";

    return copyFile(srcFile.string(), configTarget);
}
