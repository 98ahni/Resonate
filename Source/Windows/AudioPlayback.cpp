#include "AudioPlayback.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <filesystem>
#include "Base/WindowManager.h"
#include <Defines.h>
#include <Serialization/KaraokeData.h>

EM_JS(emscripten::EM_VAL, create_audio_element, (), {
    global_audio_element = new Audio();
    return Emval.toHandle(global_audio_element);
}
var global_audio_element;
if(false){
});

EM_JS(emscripten::EM_VAL, set_audio_playback_file, (emscripten::EM_VAL fs_path), {
	const audioData = FS.readFile(Emval.toValue(fs_path));
	const audioBlob = new Blob([audioData.buffer], {type: 'application/octet-binary'});
	const audioURL = URL.createObjectURL(audioBlob);
    const audio = global_audio_element;//Emval.toValue(audio_element);
    audio.src = audioURL;
    audio.preservesPitch = true;
    audio.onended = (event) => {_AudioOnEnded();};
    audio.onpause = (event) => {_AudioOnPause();};
    audio.onplay = (event) => {_AudioOnPlay();};
    audio.ontimeupdate = (event) => {_AudioOnTimeUpdate(audio.currentTime);};
    return Emval.toHandle(new Promise((resolve) => {
        audio.ondurationchange = (event) =>{
            resolve(audio.duration);
        }
    }));
});

extern"C" EMSCRIPTEN_KEEPALIVE void AudioOnEnded(){}
extern"C" EMSCRIPTEN_KEEPALIVE void AudioOnPause(){}
extern"C" EMSCRIPTEN_KEEPALIVE void AudioOnPlay(){}
extern"C" EMSCRIPTEN_KEEPALIVE void AudioOnTimeUpdate(double aTime){AudioPlayback::SetPlaybackProgress((uint)(100 * aTime)); }

EM_JS(void, audio_element_play, (), {
    global_audio_element.play();
});
EM_JS(void, audio_element_pause, (), {
    global_audio_element.pause();
});

AudioPlayback::AudioPlayback()
{
    if(ourInstance)
    {
        WindowManager::DestroyWindow(this);
        return;
    }
    myAudio = VAR_FROM_JS(create_audio_element());
}

void AudioPlayback::OnImGuiDraw()
{
    if(Gui_Begin())
    {
        if(ImGui::Button("Play"))
        {
            audio_element_play();
        }
        if(ImGui::Button("Pause"))
        {
            audio_element_pause();
        }
        uint zero = 0;
        ImGui::SliderScalar("##ProgressBar", ImGuiDataType_::ImGuiDataType_U32, &myProgress, &zero, &myDuration);
    }
    Gui_End();
}

void AudioPlayback::SetPlaybackFile(std::string aPath)
{
    printf("Loading %s.\n", aPath.c_str());
    if(!std::filesystem::exists(aPath))
    {
        printf("%s does not exist!\n", aPath.c_str());
    }
    if(std::filesystem::is_directory(aPath))
    {
        for (auto &path : std::filesystem::directory_iterator(aPath))
        {
            if (path.path().extension() == ".mp3")
            {
                SetPlaybackFile(path.path().string());
                return;
            }
        }
        printf("No audio file found!\n");
        return;
    }
    ourInstance->myDuration = 100 * VAR_FROM_JS(set_audio_playback_file(VAR_TO_JS(aPath.c_str()))).await().as<double>();
    printf("Audio duration: %s\n", Serialization::KaraokeDocument::TimeToString(ourInstance->myDuration).c_str());
}

void AudioPlayback::SetPlaybackProgress(uint aProgress)
{
    ourInstance->myProgress = aProgress;
}

uint AudioPlayback::GetPlaybackProgress()
{
    return ourInstance->myProgress;
}
