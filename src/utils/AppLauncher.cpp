#include "AppLauncher.h"
#include <windows.h>

bool restartEqualizerAPO()
{
    system("net stop \"Equalizer APO\" >nul 2>&1");
    system("net start \"Equalizer APO\" >nul 2>&1");
    return true;
}

bool launchEditor()
{
    ShellExecuteW(NULL, L"open", L"launch_editor.bat", NULL, NULL, SW_SHOWNORMAL);
    return true;
}
