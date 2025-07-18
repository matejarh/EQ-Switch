#include <iostream>
#include <conio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>
#include "resource.h"

using namespace std;


const vector<string> profiles = {
    "House",
    "Jazz",
    "Psy Trance",
    "Techno",
    "Drum & Bass",
    "Ambient",
    "Stereo Movies",
    "5.1 Movies",
    "Open Editor",
    "Exit"
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

void printMenu(int selected, const string& currentProfile, bool blinkOn) {
    system("cls");

    // Print header
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    cout << "Current Active Profile: ";
    setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    cout << currentProfile << "\n\n";

    setColor(FOREGROUND_INTENSITY);
    cout << "Use UP/DOWN arrows to navigate. Press ENTER to select.\n\n";

    for (int i = 0; i < (int)profiles.size(); ++i) {
        if (i == selected) {
            if (blinkOn) {
                // Bright yellow on blue background
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE);
            }
            else {
                // Dim yellow on blue background
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_BLUE);
            }
            cout << " > " << profiles[i] << " <" << endl;
        }
        else {
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            cout << "   " << profiles[i] << endl;
        }
    }

    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset color
}

bool copyFile(const string& src, const string& dest) {
    ifstream  source(src, ios::binary);
    ofstream  destination(dest, ios::binary);

    if (!source) return false;
    if (!destination) return false;

    destination << source.rdbuf();
    return true;
}

void restartEqualizerAPO() {
    system("net stop \"Equalizer APO\" >nul 2>&1");
    system("net start \"Equalizer APO\" >nul 2>&1");
}

bool launchEditorWithEnv(const std::wstring& exePath, const std::wstring& platformPath) {
    // Get current environment strings
    LPWCH envStrings = GetEnvironmentStringsW();
    if (!envStrings) return false;

    // Calculate environment block size
    LPWCH currentEnv = envStrings;
    size_t envSize = 0;
    while (*currentEnv) {
        size_t len = wcslen(currentEnv) + 1;
        envSize += len;
        currentEnv += len;
    }
    envSize += 1; // double null terminator

    // Copy environment to a modifiable buffer
    wchar_t* newEnv = new wchar_t[envSize];
    memcpy(newEnv, envStrings, envSize * sizeof(wchar_t));

    // Modify or add QT_QPA_PLATFORM_PLUGIN_PATH variable
    std::wstring key = L"QT_QPA_PLATFORM_PLUGIN_PATH=" + platformPath;
    bool found = false;

    wchar_t* envVar = newEnv;
    while (*envVar) {
        if (_wcsnicmp(envVar, L"QT_QPA_PLATFORM_PLUGIN_PATH=", 27) == 0) {
            // Replace existing value
            wcscpy_s(envVar, envSize, key.c_str());
            found = true;
            break;
        }
        envVar += wcslen(envVar) + 1;
    }

    // If not found, add new variable before double null terminator
    if (!found) {
        envVar = newEnv;
        while (*envVar) {
            envVar += wcslen(envVar) + 1;
        }
        wcscpy_s(envVar, envSize, key.c_str());
        envVar[key.size()] = L'\0';
        envVar[key.size() + 1] = L'\0'; // double null
    }

    // Prepare STARTUPINFO and PROCESS_INFORMATION
    STARTUPINFOW si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(si);

    // Convert exePath to modifiable wchar_t*
    wchar_t* commandLine = _wcsdup(exePath.c_str());

    // Create process with new environment block
    BOOL success = CreateProcessW(
        NULL,
        commandLine,
        NULL,
        NULL,
        FALSE,
        0,
        newEnv,
        NULL,
        &si,
        &pi
    );

    FreeEnvironmentStringsW(envStrings);
    delete[] newEnv;
    free(commandLine);

    if (!success) {
        std::wcerr << L"Failed to launch editor. Error code: " << GetLastError() << L"\n";
        return false;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

int main() {
    int selected = 0;
    int key;

    const string configDir = "C:\\Program Files\\EqualizerAPO\\config\\";
    const string profilesDir = "G:\\EQ-profiles\\";
    const string configTarget = configDir + "config.txt";

    printMenu(selected, getCurrentProfile(configTarget), false);

    while (true) {
        key = _getch();

        if (key == 224) { // Arrow keys prefix
            key = _getch();
            if (key == 72) { // UP arrow
                selected = (selected == 0) ? (profiles.size() - 1) : (selected - 1);
                printMenu(selected, getCurrentProfile(configTarget), false);
            }
            else if (key == 80) { // DOWN arrow
                selected = (selected + 1) % profiles.size();
                printMenu(selected, getCurrentProfile(configTarget), false);
            }
        }
        else if (key == 13) { // Enter key
            if (selected == (int)profiles.size() - 1) {
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                cout << "\nExiting...\n";
                break;
            }
            else if (profiles[selected] == "Open Editor") {
                // string exePath = "\"C:\\Program Files\\EqualizerAPO\\Editor.exe\"";
                // string platformPath = "C:\\Program Files\\EqualizerAPO\\platforms";
                if (launchEditorWithEnv(L"C:\\Program Files\\EqualizerAPO\\Editor.exe", L"C:\\Program Files\\EqualizerAPO\\platforms")) {
                    cout << "\nLaunched Editor successfully. Press any key to continue...";
                }
                else {
                    cout << "\nFailed to launch Editor. Press any key to continue...";
                }
                _getch();
                printMenu(selected, getCurrentProfile(configTarget), false);
            }
            else {
                string fileSuffix;
                if (profiles[selected] == "House") fileSuffix = "house";
                else if (profiles[selected] == "Jazz") fileSuffix = "jazz";
                else if (profiles[selected] == "Psy Trance") fileSuffix = "psy";
                else if (profiles[selected] == "Techno") fileSuffix = "techno";
                else if (profiles[selected] == "Drum & Bass") fileSuffix = "dnb";
                else if (profiles[selected] == "Ambient") fileSuffix = "ambient";
                else if (profiles[selected] == "Stereo Movies") fileSuffix = "movies";
                else if (profiles[selected] == "5.1 Movies") fileSuffix = "surroundmovies";

                if (fileSuffix.empty()) {
                    setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                    cout << "Unknown profile selected.\n";
                }
                else {
                    string srcFile = profilesDir + "config-" + fileSuffix + ".txt";
                    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    cout << "\nCopying " << srcFile << " to " << configTarget << " ...\n";

                    if (copyFile(srcFile, configTarget)) {
                        cout << "Copy succeeded.\nRestarting Equalizer APO service...\n";
                        restartEqualizerAPO();
                    }
                    else {
                        setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                        cout << "Failed to copy file! Make sure files exist and run as administrator.\n";
                    }
                }
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                cout << "Press any key to continue...";
                _getch();
                printMenu(selected, getCurrentProfile(configTarget), false);
            }
        }
    }
    
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    return 0;
}