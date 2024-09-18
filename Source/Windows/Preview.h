//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"
#include <Extensions/imguiExtTexture.h>
#include <unordered_map>
#include <deque>

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
    struct ImageFade
    {
        std::string myImagePath;
        uint myStartTime;
        uint myEndTime;
    };
public:
    PreviewWindow();
    void OnImGuiDraw() override;
    static void SetFont(ImFont* aFont);
    static void SetRulerFont(ImFont* aFont);
    static bool GetHasVideo();
    static void AddBackgroundElement(std::string aBGPath);
    static ImExtTexture GetBackgroundTexture(std::string aBGPath, bool aShouldReRender = false);
    static const std::vector<std::string>& GetBackgroundElementPaths();
    static void ClearBackgroundElements();

private:
    void QueueImageFade();
    int AssembleLanes(float aWidth);
    bool FillBackLanes(int aLaneCount, float aScaledWidth);
    int FillBackLanesSetLine(int aLaneCount, int aNextLineNeeds);
    bool TryDisplayLanes();
    bool CheckLaneVisible(int aLane, uint someCurrentTime, uint aDelay);
    bool RemoveOldLanes(uint someCurrentTime, uint aDelay);
    void Resetprogress();

    std::string myTexturePath;
    int myNextAddLineIndex;
    uint myPlaybackProgressLastFrame;
    std::deque<ImageFade> myBackgroundQueue;
    Lane myLanes[7];
    Lane myBackLanes[7];
    Lane myAssemblyLanes[7];
    bool myShouldDebugDraw;

    static void SaveBackgroundElementsToLocal();
    inline static ImFont* ourFont;
    static inline ImFont* ourRulerFont;
    inline static bool ourHasVideo = false;
    static inline std::vector<std::string> ourBackgroundPaths;
    inline static std::unordered_map<std::string, ImExtTexture> ourBackgrounds;
};