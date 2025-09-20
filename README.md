# 🎛️ EQ Switch

**EQ Switch** is a desktop utility for managing [**Equalizer APO**](https://sourceforge.net/projects/equalizerapo/) profiles with real-time LED-style audio visualization.  
Now featuring a **15-band Frequency VU Meter** powered by [**KissFFT**](https://github.com/mborgerding/kissfft) for live spectrum analysis.

![screenshot](screenshot.png "EQ Switch")

---

## ✨ Features

- **🎚 Profile Switching**
  - Instantly swap between [Equalizer APO](https://sourceforge.net/projects/equalizerapo/) profiles.
  - Profiles stored in a dedicated directory for easy access.

- **📊 Real-Time Audio Visualization**
  - **Stereo VU Meters**  
    - LED-style meters for left/right channels.
    - Smooth decay for natural responsiveness.
  - **Frequency VU Meter**  
    - 15 bands spanning 25 Hz → 20 kHz.
    - Log/compressive scaling — low volumes are more visible while peaks saturate at top.
    - Color zones: **Green → Yellow → Red**.
    - DPI-aware rendering for all monitor scales.

- **🖥 DPI Awareness**
  - GUI scales with `main_scale` for crisp rendering on any display.

---

## 📦 Prebuilt Profiles

The project comes with a set of prebuilt [Equalizer APO](https://sourceforge.net/projects/equalizerapo/) profiles, **primarily used for transforming stereo audio into 5.1 surround**. These profiles are located in the `eq-presets/` directory and can be customized or replaced as needed.


## Controls

- **Double-click** a profile in the list to apply
- **Use the dropdown** to switch VU meter style (Progress Bar / LED Horizontal / LED Vertical)
- **Apply button** to re-apply current profile manually
---

## 🛠️ Build Instructions

### ✅ Requirements

- Windows (x64)
- VSCode or `cl.exe` via Developer Command Prompt
- [ImGui](https://github.com/ocornut/imgui) (included locally)
- [KissFFT](https://github.com/mborgerding/kissfft) (included locally)
- [Equalizer APO](https://sourceforge.net/projects/equalizerapo/) installed

### ⚙️ VSCode Build with `tasks.json`

The project comes with a preconfigured `tasks.json` for compiling with `cl.exe`. You must have VSCode’s C++ tools installed.

**To build:**

1. Install VSCode with C++ desktop development tools
2. Clone the repository.
2. Open VSCode from the “Developer Command Prompt for VS 2022”
3. Press `Ctrl+Shift+B` to build the project
4. Run the resulting `EQSwitch.exe`

---

## 🗂️ Config Setup

Make sure your directory structure matches this:

- `eq-presets/` — contains `.txt` config profiles with a comment like `# EQ Profile: My Preset`
- `C:\Program Files\EqualizerAPO\config\config.txt` — target config for active profile

Each profile file should include a line:

```text
# EQ Profile: My Preset Name
```

## 💡 Usage

- Run the compiled executable.
- Use the profile list to select and apply an Equalizer APO profile.
- VU meters update in real-time, displaying audio levels for each channel.
- Double-click a profile to apply it.
- Click "Apply" to manually activate the selected profile.
- Press `ESC` or close the window to exit.

---

## 💡 Tips

- Add your fonts in `utils/Fonts.cpp`
- Add an icon in `resources/resource.h` and include it via `LoadIcon(...)`
- Make sure to run the app with proper permissions if needed to restart the Equalizer APO service

## 📃 License

MIT — feel free to use, modify, and redistribute.
Credit to [ImGui](https://github.com/ocornut/imgui), [Equalizer APO](https://sourceforge.net/projects/equalizerapo/) and [KissFFT](https://github.com/mborgerding/kissfft).

## 🙏 Credits

- Developed by Matej Arh
- Special thanks to the [Equalizer APO](https://sourceforge.net/projects/equalizerapo/) community, [ImGui](https://github.com/ocornut/imgui) by Omar Cornut and [KissFFT](https://github.com/mborgerding/kissfft).
- Fonts: Carlito (from Windows font directory)

## 📋 Changelog

### v1.3.0 - Frequency VU Meter
- 🎛 **Added Frequency VU Meter** — 15 bands covering 25 Hz → 20 kHz.
- 📈 **Log/Compressive Scaling** — boosts low-level details while keeping peaks natural.
- ↔️ **Layout Enhancements** — right-aligned & SameLine meter placement.
- 🖥 **DPI Scaling Improvements** — crisp visuals on all display scales.

### v1.2.0
- 🖥️ Cleaner and more responsive UI with ImGui
- 📁 Corrected profile path syntax for better compatibility
- 💡 Clear status indicators with tooltips for quick diagnostics
- 🛠 Refactored codebase for better maintainability and future updates

### v1.1.0 – Enhanced GUI and Theming
- 🆕 Added support for switching between VU meter modes: **Progress Bar**, **LED Horizontal**, and **LED Vertical**
- 🎨 Refined **Dark Modern** ImGui theme (VS Code–inspired)
- 🧱 Introduced modular **VuMeters** class
- 💡 Customizable font styles and layout improvements
- ✅ Combo box now aligns properly and uses small font
- 🛠️ Fixed profile selection issue and added auto-scroll behavior

### v1.0.0 – Initial Release
- 🎚️ VU meters with Progress Bar visualization
- 🔁 Equalizer APO profile switching with double-click
- 🖼️ Fullscreen DX11/Win32 GUI with ImGui
- 💾 Prebuilt stereo-to-5.1 Equalizer APO profiles included
- 🧩 Modular C++ architecture (AudioCapture, GUI, Config, VU, Theme)