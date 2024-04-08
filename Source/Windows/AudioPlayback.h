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

private:
    static inline AudioPlayback* ourInstance;
    emscripten::val myAudio;
    bool myHasAudio = false;
    float mySpeed;
    uint myProgress;
    uint myDuration;
};