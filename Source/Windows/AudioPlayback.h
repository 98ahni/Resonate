//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"
#include <string>

typedef unsigned int uint;
class AudioPlayback : public EditorWindow
{
public:
    enum ProcessEngine
    {
        Default,
        RubberBand,
        Browser
    };
    AudioPlayback();
    void OnImGuiDraw();

    static void PrepPlayback();
    static void SetPlaybackFile(std::string aPath);
    static ProcessEngine GetEngine();
    static void SetEngine(ProcessEngine anEngine);
    static uint GetPlaybackProgress();
    static void SetPlaybackProgress(uint someProgress);
    static int GetPlaybackSpeed();
    static bool GetIsWaitingToPlay(bool aShouldReset = false);
    static void SaveLocalBackup();
    static std::string GetPath();

private:
    static inline AudioPlayback* ourInstance;

    void ProcessAudio();
    void DrawPlaybackProgress(float aDrawUntil);
    void DrawPlaybackSpeed();

    ProcessEngine myEngine;
    bool myHasAudio = false;
    bool myWaitingToPlay = false;
    std::string myPath = "";
    int mySpeed;
    uint myProgress;
    uint myDuration;
    float myTimeScale;
};