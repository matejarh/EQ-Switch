// ProfileManager.cpp

#include <fstream>
#include <filesystem>
#include <iostream>
#include <Windows.h>
#include <sstream>
#include <imgui_internal.h>

#include "config/ProfileManager.h"
#include <imgui.h>

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
    constexpr std::string_view iconTag = "# Icon: ";

    for (const auto &entry : fs::directory_iterator(profilesDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
        {
            std::ifstream file(entry.path());
            if (!file.is_open())
                continue;

            std::string filename = entry.path().filename().string();
            std::string label;
            std::string icon = u8"\uf001"; // default icon
            std::string line;
            bool foundTag = false;
            bool foundIcon = false;

            while (std::getline(file, line))
            {
                if (!foundTag && line.rfind(tag, 0) == 0)
                {
                    label = line.substr(tag.size());
                    foundTag = true;
                }
                else if (!foundIcon && line.rfind(iconTag, 0) == 0)
                {
                    // Extract hex code and trim whitespace
                    std::string hexCode = line.substr(iconTag.size());

                    // Trim left
                    hexCode.erase(0, hexCode.find_first_not_of(" \t\r\n"));
                    // Trim right
                    hexCode.erase(hexCode.find_last_not_of(" \t\r\n") + 1);

                    // Convert to uppercase (case-insensitive handling)
                    for (char &c : hexCode)
                        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

                    // Convert to UTF-8 icon
                    unsigned int code = 0;
                    std::stringstream ss;
                    ss << std::hex << hexCode;
                    ss >> code;

                    char utf8[5] = {};
                    ImTextCharToUtf8(utf8, code);
                    icon = utf8;

                    foundIcon = true;
                }

                if (foundTag && foundIcon)
                    break; // stop reading early
            }

            if (foundTag)
            {
                profiles.push_back({label, filename, icon});
            }
        }
    }

    // Sort alphabetically by label
    std::sort(profiles.begin(), profiles.end(),
              [](const Profile &a, const Profile &b)
              {
                  return a.label < b.label;
              });
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

std::string ProfileManager::ExtractBaseLabel(const std::string &label)
{
    std::string result = label;

    // 1️⃣ Cut off everything starting from "Preset"
    const std::string presetTag = "Preset";
    size_t presetPos = result.find(presetTag);
    if (presetPos != std::string::npos)
    {
        result = result.substr(0, presetPos);
    }
    else
    {
        return result = "";
    }

    // 2️⃣ Remove "Music" if it exists in the remaining string
    const std::string musicWord = "Music";
    size_t musicPos = result.find(musicWord);
    if (musicPos != std::string::npos)
    {
        result.erase(musicPos, musicWord.length());
    }

    // 3️⃣ Trim whitespace from both ends
    auto trim = [](std::string &s)
    {
        // Left trim
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                        { return !std::isspace(ch); }));
        // Right trim
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                             { return !std::isspace(ch); })
                    .base(),
                s.end());
    };
    trim(result);

    return result;
}

std::string ProfileManager::getCurrentProfileIcon() const
{
    std::ifstream file(configTarget);
    if (!file.is_open())
    {
        return "(Could not read active profile)";
    }

    constexpr std::string_view tag = "# Icon: ";
    std::string line;
    while (std::getline(file, line))
    {
        if (line.rfind(tag, 0) == 0)
        {
            // Extract and trim
            std::string hexCode = line.substr(tag.size());
            hexCode.erase(0, hexCode.find_first_not_of(" \t\r\n"));
            hexCode.erase(hexCode.find_last_not_of(" \t\r\n") + 1);

            // Uppercase for case-insensitive handling
            for (char &c : hexCode)
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

            // Convert hex string to codepoint
            unsigned int code = 0;
            std::stringstream ss;
            ss << std::hex << hexCode;
            ss >> code;

            // Convert to UTF-8 string
            char utf8[5] = {};
            ImTextCharToUtf8(utf8, code);

            return utf8;
        }
    }

    // Fallback if not found
    return u8"\uf001";
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
