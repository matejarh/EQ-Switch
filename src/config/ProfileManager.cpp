#include "ProfileManager.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <Windows.h>

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

void ProfileManager::restartEqualizerAPO()
{
    system("net stop \"Equalizer APO\" >nul 2>&1");
    system("net start \"Equalizer APO\" >nul 2>&1");
}

bool ProfileManager::applyProfile(const Profile &p)
{
    string srcFile = profilesDir + p.filename;
    if (copyFile(srcFile, configTarget))
    {
        restartEqualizerAPO();
        return true;
    }
    return false;
}

bool ProfileManager::launchEditor()
{
    ShellExecuteW(NULL, L"open", L"launch_editor.bat", NULL, NULL, SW_SHOWNORMAL);
    return true;
}
