#include "AudioPlayback.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <filesystem>
#include "Base/WindowManager.h"
#include <Defines.h>
#include <Serialization/KaraokeData.h>
#include <Extensions/imguiExt.h>

EM_JS(emscripten::EM_VAL, create_audio_element, (), {
    global_audio_element = new Audio();
    return Emval.toHandle(global_audio_element);
}
var global_audio_element;
if(false){
});

//EM_JS(emscripten::EM_VAL, set_audio_playback_file, (emscripten::EM_VAL fs_path), {
EM_JS(void, set_audio_playback_file, (emscripten::EM_VAL fs_path), {
	const audioData = FS.readFile(Emval.toValue(fs_path));
	const audioBlob = new Blob([audioData.buffer], {type: 'application/octet-binary'});
	const audioURL = URL.createObjectURL(audioBlob);
    const audio = global_audio_element;//Emval.toValue(audio_element);
    audio.src = audioURL;
    if(audio.hasAttribute("webkitPreservesPitch"))
    {
        audio.webkitPreservesPitch = true;
    }
    else
    {
        audio.preservesPitch = true;
    }
    audio.onended = (event) => {console.log("Ended"); _AudioOnEnded();};
    audio.onpause = (event) => {console.log("Pause"); _AudioOnPause();};
    audio.onplay = (event) => {console.log("Play"); _AudioOnPlay();};
    //return Emval.toHandle(new Promise((resolve) => {
    //    global_audio_element.addEventListener("durationchange", function eventHandler(event) {
    //        this.removeEventListener("durationchange", eventHandler);
    //        resolve(audio.duration);
    //    });
    //}));
});

EM_JS(emscripten::EM_VAL, get_audio_playback_progress, (), {
    const audio = global_audio_element;
    //console.log("Get time: " + audio.currentTime);
    return Emval.toHandle(audio.currentTime);
});

EM_JS(emscripten::EM_VAL, get_audio_duration, (), {
    const audio = global_audio_element;
    return Emval.toHandle(audio.duration);
});

EM_JS(void , set_audio_playback_progress, (emscripten::EM_VAL progress), {
    const audio = global_audio_element;
    audio.currentTime = Emval.toValue(progress);
});

EM_JS(void , set_audio_playback_speed, (emscripten::EM_VAL play_rate), {
    const audio = global_audio_element;
    audio.playbackRate = Emval.toValue(play_rate);
});

extern"C" EMSCRIPTEN_KEEPALIVE void AudioOnEnded(){}
extern"C" EMSCRIPTEN_KEEPALIVE void AudioOnPause(){}
extern"C" EMSCRIPTEN_KEEPALIVE void AudioOnPlay(){}

EM_JS(void, audio_element_play, (), {
    global_audio_element.play();
    //global_audio_element.currentTime = 2;
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
    ourInstance = this;
    myAudio = VAR_FROM_JS(create_audio_element());
    myProgress = 0;
    mySpeed = 1.0;
}

void AudioPlayback::OnImGuiDraw()
{
    if(ImGui::Begin(GetName().c_str()))
    {
        myProgress = (uint)(VAR_FROM_JS(get_audio_playback_progress()).as<double>() * 100);
        if(ImGui::Button("Play"))
        {
            //myDuration = 100 * VAR_FROM_JS(get_audio_duration()).as<double>();
            audio_element_play();
        }
        ImGui::Ext::CreateHTMLButton("htmlPlay", "click", "audio_element_play");
        if(ImGui::Button("Pause"))
        {
            audio_element_pause();
        }
        ImGui::Text(Serialization::KaraokeDocument::TimeToString(myProgress).c_str());
        ImGui::SameLine();
        if(ImGui::SliderInt("##ProgressBar", (int*)&myProgress, (int)0, (int)myDuration, ""))
        {
            //myDuration = 100 * VAR_FROM_JS(get_audio_duration()).as<double>();
            set_audio_playback_progress(VAR_TO_JS(((float)myProgress) * .01f));
        }
        if(ImGui::SliderFloat("##SpeedBar", &mySpeed, 0.1f, 1.0f))
        {
            set_audio_playback_speed(VAR_TO_JS(mySpeed));
        }
    }
    Gui_End();
}

void AudioPlayback::SetPlaybackFile(std::string aPath)
{
    printf("Loading %s.\n", aPath.c_str());
    if(!std::filesystem::exists(aPath))
    {
        printf("%s does not exist!\n", aPath.c_str());
        return;
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
    set_audio_playback_file(VAR_TO_JS(aPath.c_str()));
    //ourInstance->myDuration = 100 * VAR_FROM_JS(set_audio_playback_file(VAR_TO_JS(aPath.c_str()))).await().as<double>();
    //printf("Audio duration: %s\n", Serialization::KaraokeDocument::TimeToString(ourInstance->myDuration).c_str());
}

uint AudioPlayback::GetPlaybackProgress()
{
    return ourInstance->myProgress;
}

void AudioPlayback::SetPlaybackProgress(uint someProgress)
{
    set_audio_playback_progress(VAR_TO_JS(((float)someProgress) * .01f));
}
