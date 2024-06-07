#include "Settings.h"

Settings::Settings()
{
}

Settings* Settings::ourInstance = new Settings();
Settings &Settings::Get()
{
    return *ourInstance;
}

void Settings::OnImGuiDraw()
{
    Gui_Begin();
    // Latency compensation
    // Toggle fullscreen
    // Download/Upload preferences
    // Download/Upload ImGui layout
    // Clear data
    Gui_End();
}

