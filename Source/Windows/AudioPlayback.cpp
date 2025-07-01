//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#define private public
#include "AudioPlayback.h"
#undef private
#include <emscripten.h>
#include <emscripten/val.h>
#include <filesystem>
#include <Defines.h>
#include <Serialization/KaraokeData.h>
#include <Serialization/Preferences.h>
#include <Extensions/TouchInput.h>
#include <Extensions/imguiExt.h>
#include <Extensions/FileHandler.h>
#include <Extensions/ComputeShader.h>
#include "MainWindow.h"

EM_JS(void, create_audio_element, (), {
    //global_audio_element = new Howl({src:["data:audio/wav;base64,UklGRigAAABXQVZFZm10IBIAAAABAAEARKwAAIhYAQACABAAAABkYXRhAgAAAAEA"]});
    //global_audio_element.once('playerror')
}
var global_audio_element = null;
var global_audio_context = null;
var global_audio_blobs = [];
var global_audio_completion = [];
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
    audio.onplay = (e) => { _AudioOnPlay(); };
    audio.onpause = (e) => { _AudioOnPause(); };
    audio.onended = (e) => { _AudioOnEnded(); };
    audio.ondurationchange = (e) => { _AudioOnDurationChange(); };
    window.onpagehide = (e) => {
        //global_audio_context.close();
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

extern"C" EMSCRIPTEN_KEEPALIVE void AudioOnEnded(){AudioPlayback::ourInstance->myIsPlaying = false;}
extern"C" EMSCRIPTEN_KEEPALIVE void AudioOnPause(){AudioPlayback::ourInstance->myIsPlaying = false;}
extern"C" EMSCRIPTEN_KEEPALIVE void AudioOnPlay(){AudioPlayback::ourInstance->myIsPlaying = true;AudioPlayback::ourInstance->myWaitingToPlay=false;}
extern"C" EMSCRIPTEN_KEEPALIVE void AudioOnDurationChange(){AudioPlayback::ourInstance->myDuration = 100 * AudioPlayback::ourInstance->myTimeScale * VAR_FROM_JS(get_audio_duration()).as<double>();}
extern"C" EMSCRIPTEN_KEEPALIVE bool IsWaitingToPlay(){return AudioPlayback::ourInstance->myWaitingToPlay;}

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
    myIsPlaying = false;
    myWaitingToPlay = false;
}

void AudioPlayback::OnImGuiDraw()
{
    //if(ImGui::Begin(GetName().c_str(), 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration))
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - DPI_SCALED(5));
    if(ImGui::BeginChild(GetName().c_str(), {0, 0}, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border, ImGuiWindowFlags_NoNav))
    {
        if(!TouchInput_HasTouch())
        {
            ImGui::Spacing();   // This is too much space on phones.
        }
        ImGui::Spacing();
        if(myHasAudio && !myWaitingToPlay)
        myProgress = (uint)(VAR_FROM_JS(get_audio_playback_progress()).as<double>() * 100 * myTimeScale);
        if(myWaitingToPlay || !myHasAudio) ImGui::BeginDisabled();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, ImGui::GetStyle().FramePadding.y));
        if(ImGui::Button(myHasAudio ? (myWaitingToPlay ? "Loading" : (myIsPlaying ? "Pause" : "Play")) : "Interact", {DPI_SCALED(60), 0}))
        {
            if(!myIsPlaying)
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
            else
            {
                audio_element_pause();
            }
        }
        //ImGui::Ext::CreateHTMLButton("htmlPlay", "touchstart", "audio_element_play");
        ImGui::SameLine();
        if(ImGui::Button(myHasAudio ? (myWaitingToPlay ? "..." : "Stop") : "to start", {DPI_SCALED(60), 0}))
        {
            audio_element_pause();
            SetPlaybackProgress(0);
        }
        ImGui::PopStyleVar();
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
        ImGui::Spacing();
        ImGui::Spacing();
        if(TouchInput_HasTouch())
        {
            ImGui::Spacing();   // Leave extra space for the 'Home bar' gestures.
        }
    }
    MainWindow::DockSizeOffset = ImVec2(0, (ImGui::GetWindowContentRegionMax().y + (ImGui::GetStyle().WindowPadding.y * 2)) - DPI_SCALED(6));
    //Gui_End();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

extern"C" EMSCRIPTEN_KEEPALIVE void jsPrepPlayback()
{
    AudioPlayback::PrepPlayback();
}
void AudioPlayback::PrepPlayback()
{
    if(!ourInstance || ourInstance->myHasAudio) return;
    EM_ASM(if(global_audio_context !== null)global_audio_context.close(););
    create_audio_playback();
    if(ourInstance->myPath.empty()) 
    {
        SetPlaybackFile("/local");
    }
    ourInstance->myIsPlaying = false;
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
    ourInstance->myPath = aPath;
    
    ourInstance->ProcessAudio();
    
    SaveLocalBackup();
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
    if(!ourInstance->myHasAudio) {return;}
    set_audio_playback_progress(VAR_TO_JS(((float)someProgress) * .01f / ourInstance->myTimeScale));
}

int AudioPlayback::GetPlaybackSpeed()
{
    return ourInstance->mySpeed;
}

void AudioPlayback::SetPlaybackSpeed(int aSpeed)
{
    aSpeed = aSpeed < 1 ? 1 : aSpeed > 10 ? 10 : aSpeed;
    if(ourInstance->mySpeed == aSpeed) { return; }
    ourInstance->mySpeed = aSpeed;
    ourInstance->myWantToSetSpeed = true;
}

void AudioPlayback::Play()
{
    if(!ourInstance->myHasAudio) { return; }
    if(VAR_FROM_JS(is_audio_stretched(VAR_TO_JS(ourInstance->mySpeed))).as<bool>())
    {
        ourInstance->myDuration = 100 * ourInstance->myTimeScale * VAR_FROM_JS(get_audio_duration()).as<double>();
        audio_element_play();
    }
    else
    {
        ourInstance->myWaitingToPlay = true;
        ImGui::BeginDisabled();
    }
}

void AudioPlayback::Pause()
{
    audio_element_pause();
}

void AudioPlayback::Stop()
{
    audio_element_pause();
    SetPlaybackProgress(0);
}

bool AudioPlayback::GetIsPlaying()
{
    return ourInstance->myIsPlaying;
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
        //FileHandler::SyncLocalFS();
    }
}

std::string AudioPlayback::GetPath()
{
    return ourInstance->myPath;
}

void AudioPlayback::AddEventListener(std::string anEvent, std::string aJSFunctonName)
{
    EM_ASM({global_audio_element.addEventListener(Emval.toValue($0), window[Emval.toValue($1)], true);}, VAR_TO_JS(anEvent), VAR_TO_JS(aJSFunctonName));
}

void AudioPlayback::RemoveEventListener(std::string anEvent, std::string aJSFunctonName)
{
    EM_ASM({global_audio_element.removeEventListener(Emval.toValue($0), window[Emval.toValue($1)], true);}, VAR_TO_JS(anEvent), VAR_TO_JS(aJSFunctonName));
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
    var audioWorker = new Worker('plugins/audiostretchworker.js');
    audioWorker.postMessage(global_audio_worker_setup_data);
    audioWorker.postMessage(['Work', useCrude ? crudeEngine : fineEngine, stretchIndex, useCrude]);
    audioWorker.onmessage = (result) => {
        if(useCrude){get_audio_samples_hybrid(Emval.toHandle(stretchIndex), Emval.toHandle(''), Emval.toHandle(fineEngine));}
        global_audio_blobs[result.data[1] - 1] = result.data[0];
        global_audio_completion[result.data[1] - 1] = true;
        _jsUpdateAudioBuffer(Emval.toHandle(result.data[1]));
        audioWorker.postMessage(['Revive']);
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
EM_JS(void, get_audio_samples_GPU, (emscripten::EM_VAL stretch_index, emscripten::EM_VAL kernel_name), {
    const kernelName = Emval.toValue(kernel_name);
    const stretchIndex = Emval.toValue(stretch_index);
    var audioBuff = Module['ShaderBuffer'].Create();
    var stretchedBuff = Module['ShaderBuffer'].Create();
    const audioLength = global_audio_worker_setup_data[1][0].length;
    const stretchedLength = Math.ceil(audioLength / (stretchIndex * .1));
    var compute = Module['ComputeShader'].Load('Shaders/ResonateStretchCS.wgsl');
    compute.SetBuffer('audioBuffer', audioBuff);
    compute.SetBuffer('stretchedBuffer', stretchedBuff);
    compute.SetInt("audioLength", audioLength);
    compute.SetInt("stretchedLength", stretchedLength);
    const stftBins = (
                    5 < stretchIndex ? 8192 :(
                    4 < stretchIndex ? 6140 :(
                    3 < stretchIndex ? 4096 :
                    1576)));
    compute.SetInt("stftBins", stftBins);
    const stftHop = 1 / (
                    5 < stretchIndex ? 3 :(
                    4 < stretchIndex ? 4.8 :(
                    3 < stretchIndex ? 5 :
                    6)));
    compute.SetFloat("stftHop", stftHop);
    compute.SetFloat("stretchFactor", 1 / (stretchIndex * 0.1));
    var output = [];
    new Promise(async(resolveAll)=>{
        for(var ch = 0; ch < global_audio_worker_setup_data[1].length; ch++)
        {
            await new Promise((resolve)=>{
                audioBuff.SetData(global_audio_worker_setup_data[1][ch]);
                stretchedBuff.SetData(new Float32Array(stretchedLength));
                compute.Dispatch(kernelName, (audioLength - stftBins) / (stftBins * stftHop), 1, 1);
                stretchedBuff.GetDataAsync((buffer)=>{
                    output[ch] = new Float32Array(buffer);
                    resolve();
                });
            });
        }
        resolveAll();
    }).then(()=>{
        global_audio_blobs[stretchIndex - 1] = Module.audioDataArrayToBlob(output , global_audio_worker_setup_data[3]);
        global_audio_completion[stretchIndex - 1] = true;
        _jsUpdateAudioBuffer(Emval.toHandle(stretchIndex));
        audioBuff.delete();
        stretchedBuff.delete();
        compute.delete();
    });
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
        if(_IsWaitingToPlay()){
            audio_element_play();
        }
        var audioDatas = [];
        audioDatas.length = buffer.numberOfChannels;
        for(var i = 0; i < buffer.numberOfChannels; i++){
            audioDatas[i] = buffer.getChannelData(i);
        }
        global_audio_worker_setup_data = ['Setup', audioDatas, audioDatas[0].length, buffer.sampleRate, isSafari];
    });
});

void AudioPlayback::ProcessAudio()
{
    get_audio_samples_setup(VAR_TO_JS(myPath));
    set_audio_playback_buffer(VAR_TO_JS(10));
}

EM_ASYNC_JS(emscripten::EM_VAL, get_audio_volume_db, (), {
    return Emval.toHandle(new Promise(async(resolve)=>{
        var output = {};
        output.peak = 0;
        output.mean = 0;
        output.clip_count = 0;
        if(!global_audio_completion[9]){
            resolve(output);
            return;
        }
        global_audio_context.decodeAudioData(await global_audio_blobs[9].arrayBuffer(), (buffer)=>{
            for(let ind = 0; ind < buffer.numberOfChannels; ind++){
                for(const sample of buffer.getChannelData(ind)){
                    if(1 < Math.abs(sample)) { output.clip_count++; }
                    else if(output.peak < Math.abs(sample)) { output.peak = Math.abs(sample); }
                    output.mean += Math.abs(sample);
                }
                //let data = new Float32Array(buffer.getChannelData(ind));
                //data.sort((a, b)=>{return Math.abs(a) - Math.abs(b);});
                //output.peak += Math.abs(data[Math.floor(data.length * .99)]);
                //output.mean += Math.abs(data[Math.floor(data.length * .5)]);
            }
            output.mean /= buffer.getChannelData(0).length * buffer.numberOfChannels;
            output.clip_count /= buffer.getChannelData(0).length * buffer.numberOfChannels;
            output.peak = 20 * Math.log10(output.peak/* / buffer.numberOfChannels*/);
            output.mean = 20 * Math.log10(output.mean/* / buffer.numberOfChannels*/);
            resolve(output);
        });
    }));
});

std::tuple<float, float, float> AudioPlayback::GetVolumeDB()
{
    emscripten::val volume = VAR_FROM_JS(get_audio_volume_db()).await();
    return {volume["peak"].as<float>(), volume["mean"].as<float>(), volume["clip_count"].as<float>()};
}

void AudioPlayback::DrawPlaybackProgress(float aDrawUntil)
{
    ImGui::Text(Serialization::KaraokeDocument::TimeToString(myProgress).c_str());
    ImGui::SameLine();
    float width = aDrawUntil - ImGui::GetCursorPosX();
    bool disable = myWaitingToPlay || !myHasAudio;
    if(disable) ImGui::BeginDisabled();
    ImGui::SetNextItemWidth(width - 20);
    if(ImGui::SliderInt("##ProgressBar", (int*)&myProgress, (int)0, (int)myDuration, "", ImGuiSliderFlags_NoInput) && myHasAudio)
    {
        myDuration = 100 * myTimeScale * VAR_FROM_JS(get_audio_duration()).as<double>();
        set_audio_playback_progress(VAR_TO_JS(((float)myProgress) * .01f / myTimeScale));
    }
    if(disable) ImGui::EndDisabled();
}

void AudioPlayback::DrawPlaybackSpeed()
{
    ImGui::Text("%i0%%", mySpeed);
    ImGui::SameLine();
    float width = ImGui::GetWindowSize().x - ImGui::GetCursorPosX();
    bool disable = (myWaitingToPlay && !mySelectingSpeed) || !myHasAudio;
    if(disable) ImGui::BeginDisabled();
    ImGui::SetNextItemWidth(width - 20);
    if((ImGui::SliderInt("##SpeedBar", &mySpeed, 1, 10, "", ImGuiSliderFlags_NoInput) || (myWantToSetSpeed && !disable)) && myHasAudio)
    {
        if(myEngine == ProcessEngine::Browser)
        {
            myTimeScale = 1;
            set_audio_playback_speed(VAR_TO_JS(((float)mySpeed) * .1f));
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
    mySelectingSpeed = ImGui::IsItemActive();
    if((ImGui::IsItemDeactivatedAfterEdit() || (myWantToSetSpeed && !disable)) && EM_ASM_INT(return global_audio_completion[($0) - 1] ? 1 : 0;, mySpeed) == false)
    {
        if(myEngine == ProcessEngine::Default)
        {
            get_audio_samples_hybrid(VAR_TO_JS(mySpeed), VAR_TO_JS(""), VAR_TO_JS("VexWarp"));
        }
        else if(myEngine == ProcessEngine::RubberBand)
        {
            get_audio_samples_hybrid(VAR_TO_JS(mySpeed), VAR_TO_JS("VexWarp"), VAR_TO_JS("RubberBand"));
        }
        else if(myEngine == ProcessEngine::Browser)
        {
            //get_audio_samples_hybrid(VAR_TO_JS(mySpeed), VAR_TO_JS("VexWarp"), VAR_TO_JS("RubberBand"));
        }
        else if(myEngine == ProcessEngine::GPU)
        {
            //ComputeShader cs = ComputeShader::Load("plugins/ResonateStretchCS.wgsl");
            //ShaderBuffer audioBuff = ShaderBuffer::Create();
            //cs.SetBuffer("audioBuffer", audioBuff);
            //cs.Dispatch("stretchAudio", /* numSamples / (stftHop * stftBins) */ 512);
            get_audio_samples_GPU(VAR_TO_JS(mySpeed), VAR_TO_JS("stretchAudio"));
        }
    }
    myWantToSetSpeed = false;
    if(disable) ImGui::EndDisabled();
}
