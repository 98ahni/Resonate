//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"

typedef void* ImTextureID;
typedef unsigned int uint;
struct ImFont;
class PreviewWindow : public EditorWindow
{
    struct TextureProxy
    {
    private:
        ImTextureID _id = 0;
        ImTextureID _handle = 0;
    };
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
    static void AddBackgroundElement(std::string aBGPath);
    static void ClearBackgroundElements();

private:
    int AssembleLanes(float aWidth);
    bool FillBackLanes(int aLaneCount, float aScaledWidth);
    bool TryDisplayLanes();
    bool CheckLaneVisible(int aLane, uint someCurrentTime, uint aDelay);
    bool RemoveOldLanes(uint someCurrentTime, uint aDelay);
    void Resetprogress();

    TextureProxy myTexture;
    int myNextAddLineIndex;
    uint myPlaybackProgressLastFrame;
    Lane myLanes[7];
    Lane myBackLanes[7];
    Lane myAssemblyLanes[7];
    bool myHasVideo;
    bool myShouldDebugDraw;

    static void SaveBackgroundElementsToLocal();
    static inline ImFont* ourFont;
    inline static std::vector<std::string> ourBackgroundPaths;
};