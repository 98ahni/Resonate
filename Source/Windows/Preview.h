//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"
#include <Serialization/KaraokeData.h>

typedef void* ImTextureID;
class PreviewWindow : public EditorWindow
{
public:
    PreviewWindow();
    void OnImGuiDraw() override;
    static void SetFont(ImFont* aFont);

private:
    int CalculateLanesNeeded(float aWidth);
    bool FillBackLanes(int aLaneCount);
    bool TryDisplayLanes();

    ImTextureID myTexture;
    int myNextDisplayLineIndex;
    int myNextAddLineIndex;
    int myLanes[7];
    int myBackLanes[7];

    static inline ImFont* ourFont;
};