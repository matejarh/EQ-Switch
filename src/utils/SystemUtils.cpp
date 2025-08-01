#include <filesystem>

bool IsEqualizerAPOInstalled()
{
    const std::wstring path = L"C:\\Program Files\\EqualizerAPO\\Editor.exe";
    return std::filesystem::exists(path);
}
