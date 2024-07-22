//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "AudioPlayback.h"
#include <emscripten.h>
#include <emscripten/val.h>
//#include <emscripten/wasm_worker.h>
#include <filesystem>
#include "Base/WindowManager.h"
#include <Defines.h>
#include <Serialization/KaraokeData.h>
#include <Extensions/imguiExt.h>
#include <Extensions/FileHandler.h>

EM_JS(void, create_audio_element, (), {
    //global_audio_element = new Howl({src:["data:audio/wav;base64,UklGRigAAABXQVZFZm10IBIAAAABAAEARKwAAIhYAQACABAAAABkYXRhAgAAAAEA"]});
    //global_audio_element.once('playerror')
}
var global_audio_element = null;
var global_audio_context = null;
var global_audio_blobs = [];
if(false){
});

//EM_JS(void, set_audio_playback_file, (emscripten::EM_VAL fs_path), {
//	const audioData = FS.readFile(Emval.toValue(fs_path));
//	//const audioBlob = new Blob([audioData.buffer], {type: 'application/octet-binary'});
//	const audioBlob = new Blob([audioData.buffer], {type: 'audio/mp3'});
//	const audioURL = URL.createObjectURL(audioBlob);
//    global_audio_element = new Audio();
//    const audio = global_audio_element;
//    const AudioContext = window.AudioContext || window.webkitAudioContext;
//    const audioCtx = new AudioContext();
//    try
//    {
//        audio.srcObject = audioBlob;
//    }
//    catch (e)
//    {
//        audio.src = audioURL;
//    }
//    const track = audioCtx.createMediaElementSource(global_audio_element);
//    track.connect(audioCtx.destination);
//    if(audio.hasAttribute("webkitPreservesPitch"))
//    {
//        audio.webkitPreservesPitch = true;
//    }
//    else
//    {
//        audio.preservesPitch = true;
//    }
//    console.log(audioCtx.state);   // This console.log() is necessary!
//    audioCtx.resume().then(()=>{
//        audio.pause();
//    });
//    audio.play();
//    audio.onended = (event) => {console.log("Ended"); _AudioOnEnded();};
//    audio.onpause = (event) => {console.log("Pause"); _AudioOnPause();};
//    audio.onplay = (event) => {console.log("Play"); _AudioOnPlay();};
//});

EM_ASYNC_JS(void, set_audio_playback_file, (emscripten::EM_VAL fs_path), {
	const audioData = FS.readFile(Emval.toValue(fs_path));
    const audioBlob = new Blob([audioData.buffer], {type: 'audio/mp3'});
    global_audio_blobs.length = 10;
    //const blobBuffer = await audioBlob.arrayBuffer();
    global_audio_context.decodeAudioData(await audioBlob.arrayBuffer(), (buffer) => {
        const isSafari = !!window['safari'] && safari !== 'undefined';
        global_audio_blobs[9] = Module.audioBufferToBlob(buffer, buffer.sampleRate);
        set_audio_playback_buffer(Emval.toHandle(10));
        var audioDatas = [];
        audioDatas.length = buffer.numberOfChannels;
        for(var i = 0; i < buffer.numberOfChannels; i++){
            audioDatas[i] = buffer.getChannelData(i);
        }
        const worker = new Worker('plugins/audiostretchworker.js');
        worker.postMessage([audioDatas, audioDatas[0].length, buffer.sampleRate, isSafari]);
        worker.onmessage = (result) => {
            global_audio_blobs[result.data[1] - 1] = result.data[0];
        };
    });
});

EM_JS(void, set_audio_playback_buffer, (emscripten::EM_VAL rate_index), {
    if(global_audio_blobs.length == 0) {
        return;
    }
    const audio = global_audio_element;
    try
    {
        audio.srcObject = global_audio_blobs[Emval.toValue(rate_index) - 1];
    }
    catch (e)
    {
        audio.src = URL.createObjectURL(global_audio_blobs[Emval.toValue(rate_index) - 1]);
    }
});

EM_JS(void, create_audio_playback, (), {
    global_audio_element = new Audio();
    const audio = global_audio_element;
    const AudioContext = window.AudioContext || window.webkitAudioContext;
    global_audio_context = new AudioContext();
    const track = global_audio_context.createMediaElementSource(global_audio_element);
    track.connect(global_audio_context.destination);
    audio.src = "data:audio/wav;base64,UklGRigAAABXQVZFZm10IBIAAAABAAEARKwAAIhYAQACABAAAABkYXRhAgAAAAEA";
    if(audio.hasAttribute("webkitPreservesPitch"))
    {
        audio.webkitPreservesPitch = false;
    }
    else
    {
        audio.preservesPitch = false;
    }
    console.log(global_audio_context.state);   // This console.log() is necessary!
    global_audio_context.resume();
    audio.play().then(()=>{
        audio.pause();
    });
    window.onpagehide = (e) => {
        global_audio_context.close();
    };
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
    myDuration = 0;
    mySpeed = 10;
    myTimeScale = 1;
    myHasAudio = false;
}

void AudioPlayback::OnImGuiDraw()
{
    if(ImGui::Begin(GetName().c_str(), 0, ImGuiWindowFlags_NoNav))
    {
        if(myHasAudio)
        myProgress = (uint)(VAR_FROM_JS(get_audio_playback_progress()).as<double>() * 100 * myTimeScale);
        if(ImGui::Button("Play") && myHasAudio)
        {
            myDuration = 100 * myTimeScale * VAR_FROM_JS(get_audio_duration()).as<double>();
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
    //if(ourInstance->myHasAudio || ourInstance->myPath.empty()) return;
    EM_ASM(if(global_audio_context !== null)global_audio_context.close(););
    create_audio_playback();
    //set_audio_playback_file(VAR_TO_JS(ourInstance->myPath.c_str()));
    if(ourInstance->myPath.empty()) 
    {
        SetPlaybackFile("/local");
    }
    ourInstance->myHasAudio = true;
}

void AudioPlayback::SetPlaybackFile(std::string aPath)
{
    if(aPath == ourInstance->myPath)
    {
        printf("%s is already loaded!\n", aPath.c_str());
        return;
    }
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
    std::error_code ferr;
    std::filesystem::remove("/local/" + std::filesystem::path(ourInstance->myPath).filename().string(), ferr);
    //EM_ASM(if(global_audio_context != 'undefined')global_audio_context.close(););
    //ourInstance->myHasAudio = false;
    ourInstance->myPath = aPath;
    set_audio_playback_file(VAR_TO_JS(aPath.c_str()));
    
    SaveLocalBackup();

    //ourInstance->myDuration = 100 * VAR_FROM_JS(set_audio_playback_file(VAR_TO_JS(aPath.c_str()))).await().as<double>();
    //printf("Audio duration: %s\n", Serialization::KaraokeDocument::TimeToString(ourInstance->myDuration).c_str());
}

uint AudioPlayback::GetPlaybackProgress()
{
    return ourInstance->myProgress;
}

void AudioPlayback::SetPlaybackProgress(uint someProgress)
{
    set_audio_playback_progress(VAR_TO_JS(((float)someProgress) * .01f / ourInstance->myTimeScale));
}

void AudioPlayback::SaveLocalBackup()
{
    if(!ourInstance->myPath.contains("local"))
    {
        std::filesystem::copy(ourInstance->myPath, "/local", std::filesystem::copy_options::overwrite_existing);
        FileHandler::SyncLocalFS();
    }
}

std::string AudioPlayback::GetPath()
{
    return ourInstance->myPath;
}

void AudioPlayback::DrawPlaybackProgress(float aDrawUntil)
{
    ImGui::Text(Serialization::KaraokeDocument::TimeToString(myProgress).c_str());
    ImGui::SameLine();
    float width = aDrawUntil - ImGui::GetCursorPosX();
    ImGui::SetNextItemWidth(width - 20);
    if(ImGui::SliderInt("##ProgressBar", (int*)&myProgress, (int)0, (int)myDuration, "", ImGuiSliderFlags_NoInput) && myHasAudio)
    {
        myDuration = 100 * myTimeScale * VAR_FROM_JS(get_audio_duration()).as<double>();
        set_audio_playback_progress(VAR_TO_JS(((float)myProgress) * .01f / myTimeScale));
    }
}

void AudioPlayback::DrawPlaybackSpeed()
{
    ImGui::Text("%i0%%", mySpeed);
    ImGui::SameLine();
    float width = ImGui::GetWindowSize().x - ImGui::GetCursorPosX();
    ImGui::SetNextItemWidth(width - 20);
    if(ImGui::SliderInt("##SpeedBar", &mySpeed, 1, 10, "", ImGuiSliderFlags_NoInput) && myHasAudio)
    {
        bool isPaused = EM_ASM_INT(return global_audio_element.paused ? 1 : 0;);
        set_audio_playback_buffer(VAR_TO_JS(mySpeed > 4 ? mySpeed % 2 ? mySpeed - 1 : mySpeed : 4));
        set_audio_playback_speed(VAR_TO_JS(mySpeed >= 4 ? mySpeed % 2 ? 1.f + (1.f / ((float)mySpeed - 1.f)) : 1 : ((float)mySpeed / 4.f)));
        myTimeScale = (float)(mySpeed > 4 ? mySpeed % 2 ? mySpeed - 1 : mySpeed : 4) * .1f;
        set_audio_playback_progress(VAR_TO_JS(((float)myProgress) * .01f / myTimeScale));
        if(!isPaused) audio_element_play();
    }
}
