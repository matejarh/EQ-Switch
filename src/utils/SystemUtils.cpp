#include <filesystem>

bool IsEqualizerAPOInstalled()
{
    const std::wstring path = L"C:\\Program Files\\EqualizerAPO\\Editor.exe";
    return std::filesystem::exists(path);
}

bool IsProfilesFolderAccessible()
{
    const std::wstring path = L"..\\eq-presets\\";
    return std::filesystem::exists(path);
}

bool IsLaunchEditorBatAvailable()
{
    const std::wstring path = L"launch_editor.bat";
    return std::filesystem::exists(path);
}
