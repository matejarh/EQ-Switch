#include "AppLauncher.h"
#include <windows.h>

bool launchEditor()
{
    ShellExecuteW(NULL, L"open", L"launch_editor.bat", NULL, NULL, SW_SHOWNORMAL);
    return true;
}
