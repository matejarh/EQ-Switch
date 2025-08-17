#include "fonts.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

// Global variables
ImFont* g_DefaultFont    = nullptr;
ImFont* g_BigFont    = nullptr;
ImFont* g_SmallFont      = nullptr;
ImFont* g_SmallerFont      = nullptr;
ImFont* g_SmallFontMono  = nullptr;
ImFont* g_LabelFont  = nullptr;
ImFont* g_unicodeFont    = nullptr;
ImFont* g_SmallVuLabels  = nullptr;
ImFont* g_fontAwesome  = nullptr;

static fs::path FindFontsDir()
{
    // Start from where the executable is running
    fs::path current = fs::current_path();

    // Search upward until we find "fonts" directory
    while (true)
    {
        fs::path candidate = current / "src" / "utils" / "fonts";
        if (fs::exists(candidate) && fs::is_directory(candidate))
        {
            return candidate;
        }

        candidate = current / "fonts";
        if (fs::exists(candidate) && fs::is_directory(candidate))
        {
            return candidate;
        }

        if (current.has_parent_path())
        {
            current = current.parent_path();
        }
        else
        {
            break; // Not found
        }
    }

    return {};
}

void LoadFonts(float baseFontSize)
{
    ImGuiIO& io = ImGui::GetIO();

    
    fs::path fontsDir = FindFontsDir();
    if (fontsDir.empty())
    {
        std::cerr << "❌ Could not locate fonts directory!\n";
        return;
    }
    
    auto loadFont = [&](const char* filename, float size) -> ImFont*
    {
        fs::path fontPath = fontsDir / filename;
        if (!fs::exists(fontPath))
        {
            std::cerr << "❌ Font file not found: " << fontPath << "\n";
            return nullptr;
        }
        return io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), size);
    };

        // Default font first
    g_DefaultFont = loadFont("Carlito-Regular.ttf", baseFontSize);
    if (!g_DefaultFont)
    {
        std::cerr << "⚠ Failed to load g_DefaultFont, falling back to built-in font.\n";
        g_DefaultFont = io.Fonts->AddFontDefault();
    }
    io.FontDefault = g_DefaultFont; // Make sure this is the default

    // Load fonts
    g_SmallFontMono  = loadFont("consola.ttf", 16.0f);
    g_SmallVuLabels  = loadFont("consola.ttf", 12.0f);
    g_BigFont      = loadFont("Carlito-Bold.ttf", 24.0f);
    g_SmallFont      = loadFont("Carlito-Regular.ttf", 16.0f);
    g_LabelFont      = loadFont("Carlito-Regular.ttf", 18.0f);
    g_SmallerFont      = loadFont("Carlito-Regular.ttf", 12.0f);
    g_unicodeFont    = loadFont("seguisym.ttf", 18.0f);
    g_fontAwesome    = loadFont("Font Awesome 7 Free-Solid-900.otf", baseFontSize);

    // Debug output
    if (!g_BigFont)      std::cerr << "⚠ Failed to load g_BigFont!\n";
    if (!g_SmallFont)      std::cerr << "⚠ Failed to load g_SmallFont!\n";
    if (!g_SmallerFont)      std::cerr << "⚠ Failed to load g_SmallerFont!\n";
    if (!g_SmallFontMono)  std::cerr << "⚠ Failed to load g_SmallFontMono!\n";
    if (!g_SmallVuLabels)  std::cerr << "⚠ Failed to load g_SmallVuLabels!\n";
    if (!g_LabelFont)  std::cerr << "⚠ Failed to load g_LabelFont!\n";
    if (!g_unicodeFont)    std::cerr << "⚠ Failed to load g_unicodeFont!\n";
    if (!g_fontAwesome)    std::cerr << "⚠ Failed to load g_fontAwesome!\n";
}
