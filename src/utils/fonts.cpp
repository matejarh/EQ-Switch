#include "fonts.h"
#include <iostream>

// Global variable definition
ImFont* g_SmallFont = nullptr;
ImFont* g_SmallFontMono = nullptr;

void LoadFonts(float baseFontSize)
{
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Carlito-Regular.ttf", baseFontSize);

    g_SmallFontMono = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 16.0f);

    g_SmallFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Carlito-Regular.ttf", 16.0f);

    if (!g_SmallFont)
    {
        std::cerr << "Failed to load small font!\n";
    }
}
// Example usage in your main application
// Call LoadFonts() after ImGui context is created, typically in your main loop or initialization function
// LoadFonts(20.0f); // Adjust base font size as needed
