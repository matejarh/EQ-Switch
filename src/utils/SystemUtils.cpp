#include <filesystem>

bool IsProfilesFolderAccessible(std::string profilesDir)
{
    return std::filesystem::exists(profilesDir);
}

bool editorexeIsAvailableAt(std::string editorPath)
{
    return std::filesystem::exists(editorPath);
}
