//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"

typedef void* ImTextureID;
typedef unsigned int uint;
struct ImFont;
class PreviewWindow : public EditorWindow
{
    struct Lane
    {
        int myLine = -1;
        int myStartToken = -1;
        int myEndToken = -1;
        float myWidth = 0;
    };
public:
    PreviewWindow();
    void OnImGuiDraw() override;
    static void SetFont(ImFont* aFont);

private:
    int AssembleLanes(float aWidth);
    bool FillBackLanes(int aLaneCount, float aScaledWidth);
    bool TryDisplayLanes();
    bool RemoveOldLanes(uint someCurrentTime);

    ImTextureID myTexture;
    int myNextAddLineIndex;
    Lane myLanes[7];
    Lane myBackLanes[7];
    Lane myAssemblyLanes[7];

    static inline ImFont* ourFont;
};