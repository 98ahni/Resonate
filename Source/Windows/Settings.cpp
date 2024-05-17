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
    Gui_End();
}

