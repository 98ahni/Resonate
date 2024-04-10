#include "AudioPlayback.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <filesystem>
#include "Base/WindowManager.h"
#include <Defines.h>
#include <Serialization/KaraokeData.h>
#include <Extensions/imguiExt.h>

EM_JS(void, create_audio_element, (), {
    global_audio_element = new Howl({src:["data:audio/wav;base64,UklGRigAAABXQVZFZm10IBIAAAABAAEARKwAAIhYAQACABAAAABkYXRhAgAAAAEA"]});
    //global_audio_element.once('playerror')
}
var global_audio_element;
if(false){
});

//EM_JS(emscripten::EM_VAL, set_audio_playback_file, (emscripten::EM_VAL fs_path), {
EM_JS(void, set_audio_playback_file, (emscripten::EM_VAL fs_path), {
	const audioData = FS.readFile(Emval.toValue(fs_path));
	const audioBlob = new Blob([audioData.buffer], {type: 'application/octet-binary'});
	const audioURL = URL.createObjectURL(audioBlob);
    //global_audio_element = new Howl({src:[audioURL], format:"audio/mp3"});
    const audio = global_audio_element;//Emval.toValue(audio_element);
    //audio.load();
    try
    {
        audio.srcObject = audioBlob;
    }
    catch (e)
    {
        audio.src = audioURL;
    }
    //if(audio.hasAttribute("webkitPreservesPitch"))
    //{
    //    audio.webkitPreservesPitch = true;
    //}
    //else
    //{
    //    audio.preservesPitch = true;
    //}
    audio.play().then( () =>
    {
        audio.pause();
    });
    //alert(audio.src);
    //console.log(audio.state());
    //audio.onload = (event) => {console.log("Loaded");};
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

EM_JS(void, create_audio_playback, (), {
    global_audio_element = new Audio();
    const audio = global_audio_element;
    audio.src = "data:audio/wav;base64,UklGRigAAABXQVZFZm10IBIAAAABAAEARKwAAIhYAQACABAAAABkYXRhAgAAAAEA";
    //audio.load();
    audio.play();//.then( () =>
    //{
    //    audio.pause();
    //});
    //alert(audio.src);
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
    //myAudio = VAR_FROM_JS(create_audio_element());
    //create_audio_element();
    myProgress = 0;
    mySpeed = 1.0;
    myHasAudio = false;
}

void AudioPlayback::OnImGuiDraw()
{
    if(ImGui::Begin(GetName().c_str(), 0, ImGuiWindowFlags_NoNav))
    {
        if(myHasAudio)
        myProgress = (uint)(VAR_FROM_JS(get_audio_playback_progress()).as<double>() * 100);
        if(ImGui::Button("Play") && myHasAudio)
        {
            myDuration = 100 * VAR_FROM_JS(get_audio_duration()).as<double>();
            audio_element_play();
        }
        //ImGui::Ext::CreateHTMLButton("htmlPlay", "touchstart", "audio_element_play");
        ImGui::SameLine();
        if(ImGui::Button("Pause") && myHasAudio)
        {
            audio_element_pause();
        }
        ImGui::SameLine();
        if(ImGui::GetWindowSize().x < ImGui::GetIO().DisplaySize.y) // It needs to react to windows docked next to it.
        {
            DrawPlaybackSpeed();
            DrawPlaybackProgress(ImGui::GetWindowSize().x);
        }
        else
        {
            DrawPlaybackProgress(ImGui::GetWindowSize().x * .7f);
            ImGui::SameLine();
            DrawPlaybackSpeed();
        }
    }
    Gui_End();
}

extern"C" EMSCRIPTEN_KEEPALIVE void jsPrepPlayback()
{
    AudioPlayback::PrepPlayback();
}
void AudioPlayback::PrepPlayback()
{
    if(ourInstance->myHasAudio) return;
    create_audio_playback();
    ourInstance->myHasAudio = true;
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
    ourInstance->myHasAudio = true;
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

void AudioPlayback::DrawPlaybackProgress(float aDrawUntil)
{
    ImGui::Text(Serialization::KaraokeDocument::TimeToString(myProgress).c_str());
    ImGui::SameLine();
    float width = aDrawUntil - ImGui::GetCursorPosX();
    ImGui::SetNextItemWidth(width - 20);
    if(ImGui::SliderInt("##ProgressBar", (int*)&myProgress, (int)0, (int)myDuration, "", ImGuiSliderFlags_NoInput) && myHasAudio)
    {
        myDuration = 100 * VAR_FROM_JS(get_audio_duration()).as<double>();
        set_audio_playback_progress(VAR_TO_JS(((float)myProgress) * .01f));
    }
}

void AudioPlayback::DrawPlaybackSpeed()
{
    int pertenth = 10 * mySpeed;
    ImGui::Text("%i0%%", pertenth);
    ImGui::SameLine();
    float width = ImGui::GetWindowSize().x - ImGui::GetCursorPosX();
    ImGui::SetNextItemWidth(width - 20);
    if(ImGui::SliderInt("##SpeedBar", &pertenth, 1, 10, "", ImGuiSliderFlags_NoInput) && myHasAudio)
    {
        mySpeed = pertenth * .1f;
        set_audio_playback_speed(VAR_TO_JS(mySpeed));
    }
}
