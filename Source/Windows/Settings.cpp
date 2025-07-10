//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Settings.h"
#include <filesystem>
#include <emscripten.h>
#include <emscripten/val.h>
#include <Serialization/Preferences.h>
#include <Serialization/KaraokeData.h>
#include "TimingEditor.h"
#include "AudioPlayback.h"
#include <Extensions/FileHandler.h>
#include <Extensions/imguiExt.h>
#include <Defines.h>
#include "Preview.h"
#include "MainWindow.h"

EM_JS(emscripten::EM_VAL, setup_latency_metronome, (), {
    return Emval.toHandle(new Promise(async (resolve) => {
        if(global_metronome_buffer !== null) {resolve();}
	    const audioData = FS.readFile('/Sound/Metronome.mp3');
        const audioBlob = new Blob([audioData.buffer], {type: 'audio/mp3' });
        global_audio_context.decodeAudioData(await audioBlob.arrayBuffer(), (buffer)=>{
            global_metronome_buffer = buffer;
            resolve();
        });
    }));
});
EM_JS(void, play_latency_metronome, (), {
    global_metronome_source = /*AudioPlayback*/global_audio_context.createBufferSource();
    global_metronome_source.buffer = global_metronome_buffer;
    global_metronome_source.loop = true;
    global_metronome_source.connect(/*AudioPlayback*/global_audio_context.destination);
    global_metronome_source.start();
}
var global_metronome_buffer = null;
var global_metronome_source = null;
);
EM_JS(void, stop_latency_metronome, (), {
    global_metronome_source.stop();
    global_metronome_source.disconnect();
    global_metronome_source = null;
    // TODO: Reset AudioPlayback stuff.
});
EM_JS(emscripten::EM_VAL, get_audio_context_time, (), {
    return Emval.toHandle(/*AudioPlayback*/global_audio_context.currentTime);
});

extern "C" EMSCRIPTEN_KEEPALIVE void LoadPreferences()
{
    std::filesystem::copy(FileHandler::OpenDocument("/temp", ".Resonate"), "/local/Prefs.Resonate", std::filesystem::copy_options::overwrite_existing);
    FileHandler::SyncLocalFS();
    Serialization::LoadPrefs();
}
extern "C" EMSCRIPTEN_KEEPALIVE void SavePreferences()
{
    FileHandler::DownloadDocument("/local/Prefs.Resonate", ".Resonate");
}
extern "C" EMSCRIPTEN_KEEPALIVE void LoadLayout()
{
    std::filesystem::copy_file(FileHandler::OpenDocument("/temp", ".Resonate"), "/local/Layout.Resonate", std::filesystem::copy_options::overwrite_existing);
    FileHandler::SyncLocalFS();
    ImGui::LoadIniSettingsFromDisk("/local/Layout.Resonate");
}
extern "C" EMSCRIPTEN_KEEPALIVE void SaveLayout()
{
    FileHandler::DownloadDocument("/local/Layout.Resonate", ".Resonate");
}

Settings::Settings()
{
    setup_latency_metronome();
    //myTimingEditorExists = WindowManager::GetWindow("Timing") != nullptr;
    if(!Serialization::Preferences::HasKey("Preview/UseOutline"))
    {
        Serialization::Preferences::SetBool("Preview/UseOutline", true);
    }
}

void Settings::OnImGuiDraw()
{
    Gui_Begin();
    if(!myLatencyPopup)
    {
        myLatencyPopupOpenLastFrame = false;
    }
    if(ImGui::BeginPopupModal("Auto Latency", &myLatencyPopup))
    {
        ImGui::SetWindowSize({DPI_SCALED(600), DPI_SCALED(400)}, ImGuiCond_Once);
        int latency = DrawLatencyWidget();
        ImGui::Text("Adjust the Display Latency so that the pulsing circle matches the beat.");
        ImGui::Text("Hit [Space] or tap the circle when the bass hits to adjust the Input Latency.");

        // Visualization
        float sizeY = ImGui::GetWindowHeight() - ImGui::GetCursorPosY();
        float sizeX = ImGui::GetWindowWidth();
        int timeRaw = DrawLatencyVisualization({sizeX, sizeY});
        sizeY = ImGui::GetWindowHeight() - ImGui::GetCursorPosY();
        float size = (sizeX < sizeY ? sizeX : sizeY) * .4f;

        // Latency Detection
        ImGui::SetCursorPos({(sizeX * .5f) - size, ((sizeY * .5f) + ImGui::GetCursorPosY()) - size});
        if(ImGui::InvisibleButton("##MetronomeHit", {size * 2, size * 2}) || ImGui::IsKeyPressed(ImGuiKey_Space, false))
        {
            if(timeRaw < 150)
            {
                TimingEditor::Get().SetInputLatencyOffset(timeRaw);
            }
            else
            {
                TimingEditor::Get().SetInputLatencyOffset(timeRaw - 200);
            }
        }

        ImGui::EndPopup();
    }
    else if(myLatencyPopupOpenLastFrame)
    {
        stop_latency_metronome();
        TimingEditor::Get().SetInputUnsafe(false);
    }
    // Latency compensation
    //if(!myTimingEditorExists) {ImGui::BeginDisabled();}
    DrawLatencyWidget();
    if(ImGui::Button("Open Auto Latency"))
    {
        play_latency_metronome();
        ourLatencyStartTime = VAR_FROM_JS(get_audio_context_time()).as<float>();
        ImGui::OpenPopup("Auto Latency");
        myLatencyPopup = true;
        myLatencyPopupOpenLastFrame = true;
        TimingEditor::Get().SetInputUnsafe(true);
    }
    ImGui::SeparatorText("Audio Processor");
    ImGui::BeginDisabled(!MainWindow::HasWebGPU);
    if(ImGui::RadioButton("VexWarp (GPU)", AudioPlayback::GetEngine() == AudioPlayback::GPU))
    {
        AudioPlayback::SetEngine(AudioPlayback::GPU);
    }
    ImGui::Indent();
    ImGui::BeginDisabled();
    ImGui::TextWrapped("Run the stretching on the GPU on supported devices. (Recomended)");
    if(!MainWindow::HasWebGPU) { ImGui::TextWrapped("Your browser does not support WebGPU yet. "); }
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    ImGui::Unindent();
    if(ImGui::RadioButton("VexWarp (Default)", AudioPlayback::GetEngine() == AudioPlayback::Default))
    {
        AudioPlayback::SetEngine(AudioPlayback::Default);
    }
    ImGui::Indent();
    ImGui::BeginDisabled();
    ImGui::TextWrapped("Fast and consistent on all devices. (Recomended)");
    ImGui::EndDisabled();
    ImGui::Unindent();
    if(ImGui::RadioButton("RubberBand", AudioPlayback::GetEngine() == AudioPlayback::RubberBand))
    {
        AudioPlayback::SetEngine(AudioPlayback::RubberBand);
    }
    ImGui::Indent();
    ImGui::BeginDisabled();
    ImGui::TextWrapped("The same processor used in Hibikase. \nIt's very slow and won't complete on low power devices.");
    ImGui::EndDisabled();
    ImGui::Unindent();
    if(ImGui::RadioButton("Browser", AudioPlayback::GetEngine() == AudioPlayback::Browser))
    {
        AudioPlayback::SetEngine(AudioPlayback::Browser);
    }
    ImGui::Indent();
    ImGui::BeginDisabled();
    ImGui::TextWrapped("Uses the browsers own functions. \nVery fast, but some browsers won't do a very good job.");
    ImGui::EndDisabled();
    ImGui::Unindent();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    bool previewUseOutline = Serialization::Preferences::GetBool("Preview/UseOutline");
    if(ImGui::Ext::ToggleSwitch("Outline Lyrics in Preview", &previewUseOutline))
    {
        Serialization::Preferences::SetBool("Preview/UseOutline", previewUseOutline);
    }
    ImGui::Indent();
    ImGui::BeginDisabled();
    ImGui::TextWrapped("Adds an outline to the text when viewed in the Preview window. Outlined text will not show transparency correctly. ");
    ImGui::EndDisabled();
    ImGui::Unindent();
    ImGui::Spacing();
    bool timingUseCustomFont = Serialization::Preferences::HasKey("Timing/CanUseCustomFont") && Serialization::Preferences::GetBool("Timing/CanUseCustomFont");
    if(ImGui::Ext::ToggleSwitch("Use Custom Font for Timing", &timingUseCustomFont))
    {
        Serialization::Preferences::SetBool("Timing/CanUseCustomFont", timingUseCustomFont);
    }
    ImGui::Indent();
    ImGui::BeginDisabled();
    ImGui::TextWrapped("Use the custom font (if one exists) in the Timing view. ");
    ImGui::EndDisabled();
    ImGui::Unindent();
    ImGui::Spacing();
    bool timingTokenFlash = Serialization::Preferences::HasKey("Timing/TokenFlash") && Serialization::Preferences::GetBool("Timing/TokenFlash");
    if(ImGui::Ext::ToggleSwitch("Enhance Timing Readability", &timingTokenFlash))
    {
        Serialization::Preferences::SetBool("Timing/TokenFlash", timingTokenFlash);
        TimingEditor::Get().SetTokenFlash(timingTokenFlash);
    }
    bool previewTokenFlash = Serialization::Preferences::HasKey("Preview/TokenFlash") && Serialization::Preferences::GetBool("Preview/TokenFlash");
    if(ImGui::Ext::ToggleSwitch("Enhance Preview Readability", &previewTokenFlash))
    {
        Serialization::Preferences::SetBool("Preview/TokenFlash", previewTokenFlash);
        PreviewWindow::SetTokenFlash(previewTokenFlash);
    }
    ImGui::Indent();
    ImGui::BeginDisabled();
    ImGui::TextWrapped("Gives syllables a different color at their start time. If you find it difficult to tell if the timing is right, try turning it on. ");
    ImGui::EndDisabled();
    ImGui::Unindent();
    ImGui::Spacing();
    bool shouldLoadVideo = !Serialization::Preferences::HasKey("Preview/LoadVideo") || Serialization::Preferences::GetBool("Preview/LoadVideo");
    if(ImGui::Ext::ToggleSwitch("Load Video Background", &shouldLoadVideo))
    {
        Serialization::Preferences::SetBool("Preview/LoadVideo", shouldLoadVideo);
    }
    ImGui::Indent();
    ImGui::BeginDisabled();
    ImGui::TextWrapped("Some devices may not be able load or play videos correctly. If this option is turned off when loading a project the video won't be imported. ");
    ImGui::EndDisabled();
    ImGui::Unindent();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    //if(!myTimingEditorExists) {ImGui::EndDisabled();}
    // Toggle fullscreen (might not be nessesary)
    // Download/Upload preferences
    ImGui::Button("Export Preferences");
    ImGui::Ext::CreateHTMLButton("ExportPreferences", "click", "_SavePreferences");
    ImGui::SameLine();
    ImGui::Button("Import Preferences");
    ImGui::Ext::CreateHTMLButton("ImportPreferences", "click", "_LoadPreferences");
    ImGui::Spacing();
    // Download/Upload ImGui layout
    ImGui::Button("Export Layout");
    ImGui::Ext::CreateHTMLButton("ExportLayout", "click", "_SaveLayout");
    ImGui::SameLine();
    ImGui::Button("Import Layout");
    ImGui::Ext::CreateHTMLButton("ImportLayout", "click", "_LoadLayout");
    if(!IsOpen())
    {
        ImGui::Ext::DestroyHTMLElement("ExportPreferences", 10);
        ImGui::Ext::DestroyHTMLElement("ImportPreferences", 10);
        ImGui::Ext::DestroyHTMLElement("ExportLayout", 10);
        ImGui::Ext::DestroyHTMLElement("ImportLayout", 10);
    }
    ImGui::Spacing();
    // Clear data
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, 0xFFFF0F2F);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0xFFFF1F5F);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xFFFF0F4F);
    if(ImGui::Button("DELETE SAVED DATA"))
    {
        FileHandler::ClearLocalStorage();
        EM_ASM({
            const dbname = '/local';
            var req = indexedDB.deleteDatabase(dbname);
            req.onsuccess = function() { console.log('Deleted IndexedDB /local!'); location.reload();};
            req.onerror = function() { console.error('Failed to delete IndexedDB /local!');};
            req.onblocked = function() { console.error('Failed to delete IndexedDB /local, DB was blocked!');};
        });
    }
    ImGui::PopStyleColor(3);
    Gui_End();
}

void Settings::InitLatencyVisualization()
{
    VAR_FROM_JS(setup_latency_metronome()).await();
    play_latency_metronome();
    ourLatencyStartTime = VAR_FROM_JS(get_audio_context_time()).as<float>();
}

int Settings::DrawLatencyVisualization(ImVec2 aSize)
{
    float timeRaw, time = timeRaw = (VAR_FROM_JS(get_audio_context_time()).as<float>() - ourLatencyStartTime) * 100;
    time -= TimingEditor::Get().GetRawVisualLatencyOffset();
    time = fl_mod(time, 200);
    timeRaw = fl_mod(timeRaw, 200);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float alphaMult = 1 - (fl_mod(time, 50) / 50);
    float radiusMult = (fl_mod(time, 50) + 150) / 200;
    float sizeY = aSize.y;
    float sizeX = aSize.x;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, DPI_SCALED(10)});
    ImGui::PushFont(MainWindow::Font);
    ImVec2 textSize = ImGui::CalcTextSize("Don  ka  ka  ka ");
    ImGui::SetCursorPosX((sizeX - textSize.x) * .5f);
    //ImGui::Dummy({(sizeX - textSize.x) * .5f, textSize.y});
    //ImGui::SameLine();
    uint startCol = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    Serialization::KaraokeDocument::Get().SetColor(IM_COL32_FROM_DOC(startCol), IM_COL32_FROM_DOC(startCol));
    ImGui::Ext::TimedSyllable("Don", 0, 50, timeRaw - TimingEditor::Get().GetRawVisualLatencyOffset(), false, true, false, DPI_SCALED(2), 1.4f);
    ImGui::SameLine();
    //ImGui::SetCursorPosX(((sizeX - textSize.x) * .5f) + (textSize.x * .25f));
    ImGui::Ext::TimedSyllable(" ka", 50, 70, timeRaw - TimingEditor::Get().GetRawVisualLatencyOffset(), false, true, false, DPI_SCALED(2), 1.2f);
    ImGui::SameLine();
    //ImGui::SetCursorPosX(sizeX * .5f);    //ImGui::SetCursorPosX(((sizeX - textSize.x) * .5f) + (textSize.x * .5f));
    ImGui::Ext::TimedSyllable(" ka", 100, 120, timeRaw - TimingEditor::Get().GetRawVisualLatencyOffset(), false, true, false, DPI_SCALED(2), 1.2f);
    ImGui::SameLine();
    //ImGui::SetCursorPosX(((sizeX - textSize.x) * .5f) + (textSize.x * .75f));
    ImGui::Ext::TimedSyllable(" ka", 150, 170, timeRaw - TimingEditor::Get().GetRawVisualLatencyOffset(), false, true, false, DPI_SCALED(2), 1.2f);
    //sizeY -= ImGui::GetTextLineHeightWithSpacing();
    ImGui::PopFont();
    ImGui::PopStyleVar();
    float size = (sizeX < sizeY ? sizeX : sizeY) * .4f;
    ImVec2 center = {(sizeX * .5f) + ImGui::GetWindowPos().x, (sizeY * .5f) + ImGui::GetCursorPosY() + ImGui::GetWindowPos().y};
    ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
    if(time < 51)
    {
        color.x *= 1.5f;
        color.x = color.x > 1 ? 1 : color.x;
    }
    color.w = .1f;
    drawList->AddCircleFilled(center, size * radiusMult, ImGui::ColorConvertFloat4ToU32(color));
    color.w = .3f * alphaMult;
    drawList->AddCircleFilled(center, size * .8f * radiusMult, ImGui::ColorConvertFloat4ToU32(color));
    color.w = .5f * alphaMult;
    drawList->AddCircleFilled(center, size * .6f * radiusMult, ImGui::ColorConvertFloat4ToU32(color));
    color.w = .7f * alphaMult;
    drawList->AddCircleFilled(center, size * .4f * radiusMult, ImGui::ColorConvertFloat4ToU32(color));
    color.w = .9f * alphaMult;
    drawList->AddCircleFilled(center, size * .2f * radiusMult, ImGui::ColorConvertFloat4ToU32(color));
    return timeRaw;
}

void Settings::StopLatencyVisualization()
{
    stop_latency_metronome();
}

int Settings::DrawLatencyWidget()
{
    TimingEditor& timing = TimingEditor::Get();
    int vlatency = timing.GetRawVisualLatencyOffset();
    if(ImGui::Ext::StepInt("Display Latency (cs)", vlatency, 1, 5))
    {
        timing.SetVisualLatencyOffset(vlatency);
    }
    int latency = timing.GetRawInputLatencyOffset();
    if(ImGui::Ext::StepInt("Input Latency (cs)", latency, 1, 5))
    {
        timing.SetInputLatencyOffset(latency);
    }
    return latency;
}
