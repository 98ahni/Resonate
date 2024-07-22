//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"
#include <emscripten/val.h>
#include <string>

typedef unsigned int uint;
class AudioPlayback : public EditorWindow
{
public:
    AudioPlayback();
    void OnImGuiDraw();

    static void PrepPlayback();
    static void SetPlaybackFile(std::string aPath);
    static uint GetPlaybackProgress();
    static void SetPlaybackProgress(uint someProgress);
    static void SaveLocalBackup();
    static std::string GetPath();

private:
    static inline AudioPlayback* ourInstance;

    void DrawPlaybackProgress(float aDrawUntil);
    void DrawPlaybackSpeed();

    emscripten::val myAudio;
    bool myHasAudio = false;
    std::string myPath = "";
    int mySpeed;
    uint myProgress;
    uint myDuration;
    float myTimeScale;
};