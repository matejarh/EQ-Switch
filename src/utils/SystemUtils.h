#pragma once

#include <filesystem>

bool IsEqualizerAPOInstalled();
bool IsProfilesFolderAccessible(std::string profilesDir);
bool IsLaunchEditorBatAvailable();
bool editorexeIsAvailableAt(std::string editorPath);