#include "Base/EditorWindow.h"
#include <string>

class Settings : public EditorWindow
{
public:
    Settings();
    static Settings& Get();
    void OnImGuiDraw();

private:
    static Settings* ourInstance;
};