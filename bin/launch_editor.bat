@echo off
setlocal
REM Set paths
set "apoDir=C:\Program Files\EqualizerAPO"
set "configDir=%apoDir%\config"
set "profilesDir=G:\EQ-profiles"
set "target=%configDir%\config.txt"

set "QT_QPA_PLATFORM_PLUGIN_PATH=%apoDir%\platforms"
pushd "%apoDir%"
start "" "%apoDir%\Editor.exe"
popd
