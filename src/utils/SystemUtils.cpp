#include <filesystem>

bool IsEqualizerAPOInstalled()
{
    const std::wstring path = L"C:\\Program Files\\EqualizerAPO\\Editor.exe";
    return std::filesystem::exists(path);
}

bool IsProfilesFolderAccessible(std::string profilesDir)
{
    return std::filesystem::exists(profilesDir);
}

bool IsLaunchEditorBatAvailable()
{
    const std::wstring path = L"launch_editor.bat";
    return std::filesystem::exists(path);
}

bool editorexeIsAvailableAt(std::string editorPath)
{
    return std::filesystem::exists(editorPath);
}
