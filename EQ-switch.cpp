// EQ-switch.cpp

#include <iostream>
#include <conio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include "resources/resource.h"

using namespace std;
namespace fs = std::filesystem;

const string configDir = "C:\\Program Files\\EqualizerAPO\\config\\";
const string profilesDir = "eq-presets\\";
const string configTarget = configDir + "config.txt";

struct Profile
{
    string label;
    string filename;
};

// Reads the # EQ Profile comment line from config.txt or returns "Unknown"
string getCurrentProfile(const string &configFilePath)
{
    ifstream file(configFilePath);
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

vector<Profile> loadProfiles(const string &profilesDir)
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

bool copyFile(const string &src, const string &dest)
{
    ifstream source(src, ios::binary);
    ofstream destination(dest, ios::binary);

    if (!source)
        return false;
    if (!destination)
        return false;

    destination << source.rdbuf();
    return true;
}

void restartEqualizerAPO()
{
    system("net stop \"Equalizer APO\" >nul 2>&1");
    system("net start \"Equalizer APO\" >nul 2>&1");
}

bool launchEditor()
{
    ShellExecuteW(NULL, L"open", L"launch_editor.bat", NULL, NULL, SW_SHOWNORMAL);
    return true;
}