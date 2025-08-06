#include "ProfileManager.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <Windows.h>
/* #include <Shlwapi.h> */

using namespace std;
namespace fs = std::filesystem;

ProfileManager::ProfileManager(const string &profilesDir, const string &configTarget)
    : profilesDir(profilesDir), configTarget(configTarget) {}


vector<Profile> ProfileManager::loadProfiles() const
{
    vector<Profile> profiles;

    for (const auto &entry : fs::directory_iterator(profilesDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
        {
            string filename = entry.path().filename().string();
            ifstream file(entry.path());
            string line;
            string label = filename;
            while (getline(file, line))
            {
                if (line.find("# EQ Profile: ") == 0)
                {
                    label = line.substr(14);
                    break;
                }
            }
            profiles.push_back({label, filename});
        }
    }

    return profiles;
}

string ProfileManager::getCurrentProfile() const
{
    ifstream file(configTarget);
    if (!file.is_open())
    {
        return "(Could not read active profile)";
    }
    string line;
    while (getline(file, line))
    {
        if (line.find("# EQ Profile: ") == 0)
        {
            return line.substr(14);
        }
    }
    return "(No profile comment found)";
}

bool ProfileManager::copyFile(const string &src, const string &dest)
{
    ifstream source(src, ios::binary);
    ofstream destination(dest, ios::binary);
    if (!source || !destination)
        return false;

    destination << source.rdbuf();
    return true;
}


bool ProfileManager::applyProfile(const Profile &p)
{
    std::cout << "Applying profile: " << p.label << "\n";
    string srcFile = profilesDir + p.filename;
    std::cout << "Source: " << srcFile << "\n";
    std::cout << "Target: " << configTarget << "\n";

    if (copyFile(srcFile, configTarget))
    {
        return true;
    }
    return false;
}

/* bool ProfileManager::launchEditor()
{
    ShellExecuteW(NULL, L"open", L"launch_editor.bat", NULL, NULL, SW_SHOWNORMAL);
    return true;
} */
