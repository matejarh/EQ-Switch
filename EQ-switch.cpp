#include <iostream>
#include <conio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include "resource.h"

using namespace std;
namespace fs = std::filesystem;

struct Profile {
    string label;
    string filename;
};

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void setColor(WORD color) {
    SetConsoleTextAttribute(hConsole, color);
}

// Reads the # EQ Profile comment line from config.txt or returns "Unknown"
string getCurrentProfile(const string& configFilePath) {
    ifstream file(configFilePath);
    if (!file.is_open()) {
        return "(Could not read active profile)";
    }
    string line;
    while (getline(file, line)) {
        if (line.find("# EQ Profile: ") == 0) {
            return line.substr(14);
        }
    }
    return "(No profile comment found)";
}

vector<Profile> loadProfiles(const string& profilesDir) {
    vector<Profile> profiles;

    for (const auto& entry : fs::directory_iterator(profilesDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            string filename = entry.path().filename().string();
            ifstream file(entry.path());
            string line;
            string label = filename;
            while (getline(file, line)) {
                if (line.find("# EQ Profile: ") == 0) {
                    label = line.substr(14);
                    break;
                }
            }
            profiles.push_back({ label, filename });
        }
    }

    profiles.push_back({ "Open Editor", "" });
    profiles.push_back({ "Exit", "" });

    return profiles;
}

void printMenu(int selected, int hovered, const string& currentProfile, const vector<Profile>& profiles, bool blinkOn) {
    COORD cursorPos = { 0, 0 };
    SetConsoleCursorPosition(hConsole, cursorPos);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD consoleSize = csbi.dwSize.X * csbi.dwSize.Y;
    DWORD charsWritten;
    FillConsoleOutputCharacter(hConsole, ' ', consoleSize, cursorPos, &charsWritten);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, consoleSize, cursorPos, &charsWritten);

    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    cout << "Current Active Profile: ";
    setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    cout << currentProfile << "\n\n";

    setColor(FOREGROUND_INTENSITY);
    cout << "Use UP/DOWN arrows or click to navigate. Press ENTER or click to select.\n\n";

    for (int i = 0; i < (int)profiles.size(); ++i) {
        if (i == selected && i == hovered) {
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_RED | BACKGROUND_BLUE);
            cout << " > " << profiles[i].label << " <" << endl;
        } else if (i == selected) {
            if (blinkOn) {
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE);
            } else {
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_BLUE);
            }
            cout << " > " << profiles[i].label << " <" << endl;
        } else if (i == hovered) {
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_RED);
            cout << " > " << profiles[i].label << " <" << endl;
        } else {
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            cout << "   " << profiles[i].label << endl;
        }
    }

    setColor(FOREGROUND_INTENSITY);
    cout << "\nCreated by Grower.\n";

    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

bool copyFile(const string& src, const string& dest) {
    ifstream source(src, ios::binary);
    ofstream destination(dest, ios::binary);

    if (!source) return false;
    if (!destination) return false;

    destination << source.rdbuf();
    return true;
}

void restartEqualizerAPO() {
    system("net stop \"Equalizer APO\" >nul 2>&1");
    system("net start \"Equalizer APO\" >nul 2>&1");
}

bool launchEditor() {
    ShellExecuteW(NULL, L"open", L"launch_editor.bat", NULL, NULL, SW_SHOWNORMAL);
    return true;
}

bool enableMouseInput() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prevMode;
    GetConsoleMode(hInput, &prevMode);
    return SetConsoleMode(hInput, prevMode | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
}

int getMenuIndexFromMouseY(int mouseY, int startLine, int menuSize) {
    int index = mouseY - startLine;
    if (index >= 0 && index < menuSize) {
        return index;
    }
    return -1;
}

void performSelection(const Profile& profile, const string& profilesDir, const string& configTarget) {
    const string& label = profile.label;

    if (label == "Exit") {
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "\nExiting...\n";
        exit(0);
    } else if (label == "Open Editor") {
        if (launchEditor()) {
            cout << "\nLaunched Editor successfully. Press any key to continue...";
        } else {
            cout << "\nFailed to launch Editor. Press any key to continue...";
        }
        _getch();
    } else {
        string srcFile = profilesDir + profile.filename;
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "\nCopying " << srcFile << " to " << configTarget << " ...\n";

        if (copyFile(srcFile, configTarget)) {
            cout << "Copy succeeded. Restarting Equalizer APO service...\n";
            restartEqualizerAPO();
        } else {
            setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
            cout << "Failed to copy file! Make sure files exist and run as administrator.\n";
        }
        cout << "Press any key to continue...";
        _getch();
    }
}

int main() {
    int selected = 0;
    int hovered = -1;

    const string configDir = "C:\\Program Files\\EqualizerAPO\\config\\";
    const string profilesDir = "G:\\EQ-profiles\\";
    const string configTarget = configDir + "config.txt";

    vector<Profile> profiles = loadProfiles(profilesDir);

    enableMouseInput();
    printMenu(selected, hovered, getCurrentProfile(configTarget), profiles, true);

    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD events;
    INPUT_RECORD inputRecord;

    const int menuStartLine = 6;

    while (true) {
        ReadConsoleInput(hInput, &inputRecord, 1, &events);

        if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown) {
            int key = inputRecord.Event.KeyEvent.wVirtualKeyCode;

            if (key == VK_UP) {
                selected = (selected == 0) ? (profiles.size() - 1) : (selected - 1);
            } else if (key == VK_DOWN) {
                selected = (selected + 1) % profiles.size();
            } else if (key == VK_RETURN) {
                performSelection(profiles[selected], profilesDir, configTarget);
            }
        } else if (inputRecord.EventType == MOUSE_EVENT) {
            MOUSE_EVENT_RECORD& mouse = inputRecord.Event.MouseEvent;
            int y = mouse.dwMousePosition.Y;
            int newHovered = getMenuIndexFromMouseY(y, menuStartLine, profiles.size());
            if (newHovered != hovered) {
                hovered = newHovered;
            }
            if (hovered != -1 && (mouse.dwEventFlags == 0 || mouse.dwEventFlags == DOUBLE_CLICK)) {
                if (mouse.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
                    selected = hovered;
                    performSelection(profiles[selected], profilesDir, configTarget);
                }
            }
        }

        printMenu(selected, hovered, getCurrentProfile(configTarget), profiles, true);
    }

    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    return 0;
}
