#include "Base/EditorWindow.h"
#include <emscripten/val.h>
#include <string>

typedef unsigned int uint;
class AudioPlayback : public EditorWindow
{
public:
    AudioPlayback();
    void OnImGuiDraw();

    static void SetPlaybackFile(std::string aPath);
    static void SetPlaybackProgress(uint aProgress);
    static uint GetPlaybackProgress();

private:
    static inline AudioPlayback* ourInstance;
    emscripten::val myAudio;
    uint myProgress;
    uint myDuration;
};