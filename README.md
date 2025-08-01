# 🎛️ EQ Switch

**EQ Switch** is a Windows GUI tool for managing [Equalizer APO](https://sourceforge.net/projects/equalizerapo/) configuration profiles with live audio VU meters and a smooth ImGui interface.

---

## ✨ Features

- 📁 Load multiple Equalizer APO config profiles from a folder
- 🔄 Apply profiles with a single click or double-click
- 🎚️ Real-time VU meters for up to 6 audio channels
- 📝 Launch your profile editor (via `launch_editor.bat`)
- 🖼️ Minimal fullscreen-style layout with font scaling for HiDPI

---

## 📦 Folder Structure

```
EQSwitch/
├── imgui/ # ImGui + backends (DX11/Win32)
├── src/
│ ├── main.cpp # WinMain entry and render loop
│ ├── gui/
│ │ └── EQSwitchWindow.{cpp,h}
│ ├── audio/
│ │ └── AudioCapture.{cpp,h}
│ ├── config/
│ │ └── ProfileManager.{cpp,h}
│ ├── utils/
│ │ ├── VUBuffer.{cpp,h}
│ │ └── Fonts.{cpp,h}
├── eq-presets/ # Your custom config .txt profiles
├── launch_editor.bat # Optional editor launcher
├── resources/ # Icons / resources
├── tasks.json # VSCode build tasks
└── README.md
```

---

## 🛠️ Build Instructions

### ✅ Requirements

- Windows (x64)
- Visual Studio or `cl.exe` via Developer Command Prompt
- [ImGui](https://github.com/ocornut/imgui) (included locally)
- [Equalizer APO](https://sourceforge.net/projects/equalizerapo/) installed

### ⚙️ VSCode Build with `tasks.json`

The project comes with a preconfigured `tasks.json` for compiling with `cl.exe`. You must have Visual Studio’s C++ tools installed.

**To build:**

1. Open VSCode from the “Developer Command Prompt for VS 2022”
2. Press `Ctrl+Shift+B` to build the project
3. Run the resulting `EQSwitch.exe`

---

## 🗂️ Config Setup

Make sure your directory structure matches this:

- `eq-presets/` — contains `.txt` config profiles with a comment like `# EQ Profile: My Preset`
- `C:\Program Files\EqualizerAPO\config\config.txt` — target config for active profile

Each profile file should include a line:
```text
# EQ Profile: My Preset Name
```

## 💡 Tips

- Add your fonts in `utils/Fonts.cpp`
- Add an icon in `resources/resource.h` and include it via `LoadIcon(...)`
- Make sure to run the app with proper permissions if needed to restart the Equalizer APO service

## 📃 License
MIT — feel free to use, modify, and redistribute.
Credit to [ImGui](https://github.com/ocornut/imgui) and [Equalizer APO](https://sourceforge.net/projects/equalizerapo/).

## 🙏 Credits
Developed by Matej Arh
Special thanks to the [Equalizer APO](https://sourceforge.net/projects/equalizerapo/) and [ImGui](https://github.com/ocornut/imgui) communities.