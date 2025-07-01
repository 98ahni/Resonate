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
        Browser,
        GPU
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
    static void SetPlaybackSpeed(int aSpeed);
    static void Play();
    static void Pause();
    static void Stop();
    static bool GetIsPlaying();
    static bool GetIsWaitingToPlay(bool aShouldReset = false);
    static void SaveLocalBackup();
    static std::string GetPath();
    static void AddEventListener(std::string anEvent, std::string aJSFunctonName);
    static void RemoveEventListener(std::string anEvent, std::string aJSFunctonName);
    static std::tuple<float, float, float> GetVolumeDB();

private:            // Is defined as public in AudioPlayback.cpp
    static inline AudioPlayback* ourInstance = nullptr;

    void ProcessAudio();
    void DrawPlaybackProgress(float aDrawUntil);
    void DrawPlaybackSpeed();

    ProcessEngine myEngine;
    bool myHasAudio = false;
    bool myIsPlaying = false;
    bool myWaitingToPlay = false;
    bool myWantToSetSpeed = false;
    bool mySelectingSpeed = false;
    std::string myPath = "";
    int mySpeed;
    uint myProgress;
    uint myDuration;
    float myTimeScale;
};