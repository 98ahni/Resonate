//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"
#include <string>

struct ImVec2;
class Settings : public EditorWindow
{
public:
    Settings();
    void OnImGuiDraw();
    static void InitLatencyVisualization();
    static int DrawLatencyVisualization(ImVec2 aSize);
    static void StopLatencyVisualization();

private:
    int DrawLatencyWidget();

    bool myTimingEditorExists;
    bool myLatencyPopup;
    bool myLatencyPopupOpenLastFrame;
    static inline float ourLatencyStartTime;
};