//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "AudioPlayback.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <filesystem>
#include <Defines.h>
#include <Serialization/KaraokeData.h>
#include <Serialization/Preferences.h>
#include <Extensions/imguiExt.h>
#include <Extensions/FileHandler.h>

EM_JS(void, create_audio_element, (), {
    //global_audio_element = new Howl({src:["data:audio/wav;base64,UklGRigAAABXQVZFZm10IBIAAAABAAEARKwAAIhYAQACABAAAABkYXRhAgAAAAEA"]});
    //global_audio_element.once('playerror')
}
var global_audio_element = null;
var global_audio_context = null;
var global_audio_blobs = [];
var global_audio_completion = [];
var global_audio_worker_corse;// = new Worker('plugins/audiostretchworker.js');
var global_audio_worker_fine;// = new Worker('plugins/audiostretchworker.js');
var global_audio_worker_setup_data = [];
if(false){
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

EM_JS(emscripten::EM_VAL, is_audio_stretched, (emscripten::EM_VAL rate_index), {
    return Emval.toHandle(global_audio_completion[Emval.toValue(rate_index) - 1]);
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
        audio.webkitPreservesPitch = true;
    }
    else
    {
        audio.preservesPitch = true;
    }
    default_console_log(global_audio_context.state);   // This console.log() is necessary!
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
    myEngine = Serialization::Preferences::HasKey("AudioPlayback/Engine") ?
        (ProcessEngine)Serialization::Preferences::GetInt("AudioPlayback/Engine") :
        ProcessEngine::Default;
    ourInstance = this;
    myProgress = 0;
    myDuration = 0;
    mySpeed = 10;
    myTimeScale = 1;
    myHasAudio = false;
    myWaitingToPlay = false;
}

void AudioPlayback::OnImGuiDraw()
{
    if(ImGui::Begin(GetName().c_str(), 0, ImGuiWindowFlags_NoNav))
    {
        if(myHasAudio)
        myProgress = (uint)(VAR_FROM_JS(get_audio_playback_progress()).as<double>() * 100 * myTimeScale);
        if(myWaitingToPlay || !myHasAudio) ImGui::BeginDisabled();
        if(ImGui::Button(myHasAudio ? (myWaitingToPlay ? "Loading" : "Play") : "Interact"))
        {
            if(VAR_FROM_JS(is_audio_stretched(VAR_TO_JS(mySpeed))).as<bool>())
            {
                myDuration = 100 * myTimeScale * VAR_FROM_JS(get_audio_duration()).as<double>();
                audio_element_play();
            }
            else
            {
                myWaitingToPlay = true;
                ImGui::BeginDisabled();
            }
        }
        //ImGui::Ext::CreateHTMLButton("htmlPlay", "touchstart", "audio_element_play");
        ImGui::SameLine();
        if(ImGui::Button(myHasAudio ? "Pause" : "to start"))
        {
            audio_element_pause();
        }
        if(myWaitingToPlay || !myHasAudio) ImGui::EndDisabled();
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
    //create_audio_playback_with_worklet();
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
    
    ourInstance->ProcessAudio();
    
    SaveLocalBackup();

    //ourInstance->myDuration = 100 * VAR_FROM_JS(set_audio_playback_file(VAR_TO_JS(aPath.c_str()))).await().as<double>();
    //printf("Audio duration: %s\n", Serialization::KaraokeDocument::TimeToString(ourInstance->myDuration).c_str());
}

AudioPlayback::ProcessEngine AudioPlayback::GetEngine()
{
    return ourInstance->myEngine;
}

void AudioPlayback::SetEngine(AudioPlayback::ProcessEngine anEngine)
{
    ourInstance->myEngine = anEngine;
    Serialization::Preferences::SetInt("AudioPlayback/Engine", anEngine);
    if(anEngine == ProcessEngine::Browser){set_audio_playback_buffer(VAR_TO_JS(10));}
}

uint AudioPlayback::GetPlaybackProgress()
{
    return ourInstance->myProgress;
}

void AudioPlayback::SetPlaybackProgress(uint someProgress)
{
    set_audio_playback_progress(VAR_TO_JS(((float)someProgress) * .01f / ourInstance->myTimeScale));
}

int AudioPlayback::GetPlaybackSpeed()
{
    return ourInstance->mySpeed;
}

bool AudioPlayback::GetIsWaitingToPlay(bool aShouldReset)
{
    bool output = ourInstance->myWaitingToPlay;
    if(aShouldReset) ourInstance->myWaitingToPlay = false;
    return output;
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

extern"C" EMSCRIPTEN_KEEPALIVE void jsUpdateAudioBuffer(emscripten::EM_VAL buffer_index)
{
    int ind = VAR_FROM_JS(buffer_index).as<int>();
    if(AudioPlayback::GetPlaybackSpeed() == ind)
    {
        uint progress = AudioPlayback::GetPlaybackProgress();
        float scale = (float)(AudioPlayback::GetPlaybackSpeed()) * .1f;
        bool isPaused = EM_ASM_INT(return global_audio_element.paused ? 1 : 0;);
        set_audio_playback_buffer(VAR_TO_JS(ind));
        set_audio_playback_progress(VAR_TO_JS((((float)progress) * .01f) / scale));
        if(AudioPlayback::GetIsWaitingToPlay(true) || !isPaused) audio_element_play();
    }
}
EM_JS(void, get_audio_samples_hybrid, (emscripten::EM_VAL stretch_index, emscripten::EM_VAL crude_engine, emscripten::EM_VAL fine_engine), {
    const crudeEngine = Emval.toValue(crude_engine);
    const fineEngine = Emval.toValue(fine_engine);
    const stretchIndex = Emval.toValue(stretch_index);
    var useCrude = crudeEngine !== '';
    //var audioWorker = useCrude ? global_audio_worker_corse : global_audio_worker_fine;
    var audioWorker = new Worker('plugins/audiostretchworker.js');
    audioWorker.postMessage(global_audio_worker_setup_data);
    audioWorker.postMessage(['Work', useCrude ? crudeEngine : fineEngine, stretchIndex, useCrude]);
    audioWorker.onmessage = (result) => {
        if(useCrude){get_audio_samples_hybrid(Emval.toHandle(stretchIndex), Emval.toHandle(''), Emval.toHandle(fineEngine));}
        global_audio_blobs[result.data[1] - 1] = result.data[0];
        global_audio_completion[result.data[1] - 1] = true;
        _jsUpdateAudioBuffer(Emval.toHandle(result.data[1]));
        audioWorker.postMessage(['Revive']);
        //audioWorker.onmessage = (result) => {
        //    audioWorker.terminate();
        //    if(useCrude){
        //        global_audio_worker_corse = new Worker('plugins/audiostretchworker.js');
        //        console.log('Resetting corse');
        //    }
        //    else {
        //        global_audio_worker_fine = new Worker('plugins/audiostretchworker.js');
        //        console.log('Resetting fine');
        //    }
        //    result.data[0] = 'Setup';
        //    (useCrude ? global_audio_worker_corse : global_audio_worker_fine).postMessage(result.data);
        //};
        audioWorker.onmessage = (result) => {
            audioWorker.terminate();
            if(useCrude){
                console.log('Resetting corse engine ' + result.data[0]);
            }
            else {
                console.log('Resetting fine engine ' + result.data[0]);
            }
        };
    };
});
EM_JS(void, get_audio_samples_legacy, (emscripten::EM_VAL stretch_index, emscripten::EM_VAL use_crude), {
    var useCrude = Emval.toValue(use_crude);
    var audioWorker = useCrude ? global_audio_worker_corse : global_audio_worker_fine;
    audioWorker.postMessage(['Work', 'Legacy', Emval.toValue(stretch_index), useCrude]);
    audioWorker.onmessage = (result) => {
        global_audio_blobs[result.data[1] - 1] = result.data[0];
        global_audio_completion[result.data[1] - 1] = true;
        _jsUpdateAudioBuffer(Emval.toHandle(result.data[1]));
        audioWorker.postMessage(['Revive']);
        audioWorker.onmessage = (result) => {
            audioWorker.terminate();
            if(useCrude){
                global_audio_worker_corse = new Worker('plugins/audiostretchworker.js');
                console.log('Resetting corse');
            }
            else {
                global_audio_worker_fine = new Worker('plugins/audiostretchworker.js');
                console.log('Resetting fine');
            }
            result.data[0] = 'Setup';
            (useCrude ? global_audio_worker_corse : global_audio_worker_fine).postMessage(result.data);
        };
    }
});
EM_JS(void, get_audio_samples_rubberband, (emscripten::EM_VAL stretch_index, emscripten::EM_VAL use_crude), {
    var useCrude = Emval.toValue(use_crude);
    var audioWorker = useCrude ? global_audio_worker_corse : global_audio_worker_fine;
    audioWorker.postMessage(['Work', 'RubberBand', Emval.toValue(stretch_index), useCrude]);
    audioWorker.onmessage = (result) => {
        global_audio_blobs[result.data[1] - 1] = result.data[0];
        global_audio_completion[result.data[1] - 1] = true;
        _jsUpdateAudioBuffer(Emval.toHandle(result.data[1]));
        audioWorker.postMessage(['Revive']);
        audioWorker.onmessage = (result) => {
            audioWorker.terminate();
            if(useCrude){
                global_audio_worker_corse = new Worker('plugins/audiostretchworker.js');
                console.log('Resetting corse');
            }
            else {
                global_audio_worker_fine = new Worker('plugins/audiostretchworker.js');
                console.log('Resetting fine');
            }
            result.data[0] = 'Setup';
            (useCrude ? global_audio_worker_corse : global_audio_worker_fine).postMessage(result.data);
        };
    }
});
EM_JS(void, get_audio_samples_vexwarp, (emscripten::EM_VAL stretch_index, emscripten::EM_VAL use_crude), {
    var useCrude = Emval.toValue(use_crude);
    var audioWorker = useCrude ? global_audio_worker_corse : global_audio_worker_fine;
    audioWorker.postMessage(['Work', 'VexWarp', Emval.toValue(stretch_index), useCrude]);
    audioWorker.onmessage = (result) => {
        global_audio_blobs[result.data[1] - 1] = result.data[0];
        global_audio_completion[result.data[1] - 1] = true;
        _jsUpdateAudioBuffer(Emval.toHandle(result.data[1]));
        audioWorker.postMessage(['Revive']);
        audioWorker.onmessage = (result) => {
            audioWorker.terminate();
            if(useCrude){
                global_audio_worker_corse = new Worker('plugins/audiostretchworker.js');
                console.log('Resetting corse');
            }
            else {
                global_audio_worker_fine = new Worker('plugins/audiostretchworker.js');
                console.log('Resetting fine');
            }
            result.data[0] = 'Setup';
            (useCrude ? global_audio_worker_corse : global_audio_worker_fine).postMessage(result.data);
        };
    }
});
EM_ASYNC_JS(void, get_audio_samples_setup, (emscripten::EM_VAL fs_path), {
	const audioData = FS.readFile(Emval.toValue(fs_path));
    const audioBlob = new Blob([audioData.buffer], {type: 'audio/mp3' });
    global_audio_blobs.length = 10;
    global_audio_completion = [false, false, false, false, false, false, false, false, false, false];
    global_audio_context.decodeAudioData(await audioBlob.arrayBuffer(), (buffer)=>{
        const isSafari = !!window['safari'] && safari !== 'undefined';
        global_audio_blobs[9] = Module.audioBufferToBlob(buffer, buffer.sampleRate);
        global_audio_completion[9] = true;
        set_audio_playback_buffer(Emval.toHandle(10));
        var audioDatas = [];
        audioDatas.length = buffer.numberOfChannels;
        for(var i = 0; i < buffer.numberOfChannels; i++){
            audioDatas[i] = buffer.getChannelData(i);
        }
        //global_audio_worker_corse.postMessage(['Setup', audioDatas, audioDatas[0].length, buffer.sampleRate, isSafari]);
        //global_audio_worker_fine.postMessage(['Setup', audioDatas, audioDatas[0].length, buffer.sampleRate, isSafari]);
        global_audio_worker_setup_data = ['Setup', audioDatas, audioDatas[0].length, buffer.sampleRate, isSafari];
    });
});
EM_ASYNC_JS(void, get_audio_samples_no_stretch, (emscripten::EM_VAL fs_path), {
	const audioData = FS.readFile(Emval.toValue(fs_path));
    const audioBlob = new Blob([audioData.buffer], {type: 'audio/mp3' });
    global_audio_blobs.length = 10;
    global_audio_context.decodeAudioData(await audioBlob.arrayBuffer(), (buffer)=>{
        global_audio_blobs[9] = Module.audioBufferToBlob(buffer, buffer.sampleRate);
        set_audio_playback_buffer(Emval.toHandle(10));
    });
});

void AudioPlayback::ProcessAudio()
{
    get_audio_samples_setup(VAR_TO_JS(myPath));
    set_audio_playback_buffer(VAR_TO_JS(10));
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
        if(myEngine == ProcessEngine::Browser)
        {
            myTimeScale = ((float)mySpeed) * .1f;
            set_audio_playback_speed(VAR_TO_JS(myTimeScale));
            return;
        }
        bool isPaused = EM_ASM_INT(return global_audio_element.paused ? 1 : 0;);
        set_audio_playback_buffer(VAR_TO_JS(mySpeed));
        set_audio_playback_speed(VAR_TO_JS(1));
        myTimeScale = ((float)mySpeed) * .1f;
        set_audio_playback_progress(VAR_TO_JS(((float)myProgress) * .01f / myTimeScale));
        if(!isPaused) 
        {
            if(VAR_FROM_JS(is_audio_stretched(VAR_TO_JS(mySpeed))).as<bool>())
            {
                audio_element_play();
            }
            else
            {
                myWaitingToPlay = true;
            }
        }
    }
    if(ImGui::IsItemDeactivatedAfterEdit() && EM_ASM_INT(return global_audio_completion[($0) - 1] ? 1 : 0;, mySpeed) == false)
    {
        if(myEngine == ProcessEngine::Default)
        {
            get_audio_samples_hybrid(VAR_TO_JS(mySpeed), VAR_TO_JS("Legacy"), VAR_TO_JS("VexWarp"));
        }
        else if(myEngine == ProcessEngine::RubberBand)
        {
            get_audio_samples_hybrid(VAR_TO_JS(mySpeed), VAR_TO_JS("VexWarp"), VAR_TO_JS("RubberBand"));
        }
        else if(myEngine == ProcessEngine::Browser)
        {
            //get_audio_samples_hybrid(VAR_TO_JS(mySpeed), VAR_TO_JS("VexWarp"), VAR_TO_JS("RubberBand"));
        }
    }
}
