#include "AppLauncher.h"
#include <windows.h>
#include <string>

bool launchEditor(const std::string &editorPath, const std::string &apoDir)
{
    // Convert paths to wide strings
    std::wstring wEditorPath = std::wstring(editorPath.begin(), editorPath.end());
    std::wstring wApoDir = std::wstring(apoDir.begin(), apoDir.end());

    // Setup process startup info
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    // Set environment variable QT_QPA_PLATFORM_PLUGIN_PATH
    wchar_t envVars[32767]; // Max environment block size
    GetEnvironmentStringsW(); // Can use this if you want to copy existing env
    wsprintfW(envVars, L"QT_QPA_PLATFORM_PLUGIN_PATH=%s\\platforms", wApoDir.c_str());

    // Use CreateProcessW to launch Editor.exe with working dir and env
    BOOL success = CreateProcessW(
        wEditorPath.c_str(),      // Application name
        NULL,                     // Command line
        NULL,                     // Process handle not inheritable
        NULL,                     // Thread handle not inheritable
        FALSE,                    // Set handle inheritance to FALSE
        CREATE_UNICODE_ENVIRONMENT, // Necessary for wide env block
        envVars,                  // Environment
        wApoDir.c_str(),          // Working directory
        &si,                      // Pointer to STARTUPINFO structure
        &pi                       // Pointer to PROCESS_INFORMATION structure
    );

    if (success)
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    else
    {
        MessageBoxW(NULL, L"Failed to launch Equalizer APO Editor.", L"Error", MB_ICONERROR);
        return false;
    }
}
