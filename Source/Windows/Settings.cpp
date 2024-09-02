//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Settings.h"
#include <filesystem>
#include <emscripten.h>
#include <emscripten/val.h>
#include <Serialization/Preferences.h>
#include "TimingEditor.h"
#include "AudioPlayback.h"
#include <Extensions/FileHandler.h>
#include <Extensions/imguiExt.h>
#include <Defines.h>

EM_ASYNC_JS(void, setup_latency_metronome, (), {
    if(global_metronome_buffer !== null) {return;}
	const audioData = FS.readFile('/Sound/Metronome.mp3');
    const audioBlob = new Blob([audioData.buffer], {type: 'audio/mp3' });
    global_audio_context.decodeAudioData(await audioBlob.arrayBuffer(), (buffer)=>{
        global_metronome_buffer = buffer;
    });
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
        ImGui::SetWindowSize({DPI_SCALED(400), DPI_SCALED(300)}, ImGuiCond_Once);
        int latency = DrawLatencyWidget();
        ImGui::Text("Hit [Space] or tap the circle when the bass hits.");

        // Visualization
        float timeRaw, time = timeRaw = (VAR_FROM_JS(get_audio_context_time()).as<float>() - myLatencyStartTime) * 100;
        time -= latency;
        time = fl_mod(time, 200);
        timeRaw = fl_mod(timeRaw, 200);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        float alphaMult = 1 - (fl_mod(time, 50) / 50);
        float radiusMult = (fl_mod(time, 50) + 150) / 200;
        float sizeY = ImGui::GetWindowHeight() - ImGui::GetCursorPosY();
        float sizeX = ImGui::GetWindowWidth();
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

        // Latency Detection
        ImGui::SetCursorPos({(sizeX * .5f) - size, ((sizeY * .5f) + ImGui::GetCursorPosY()) - size});
        if(ImGui::InvisibleButton("##MetronomeHit", {size * 2, size * 2}) || ImGui::IsKeyPressed(ImGuiKey_Space, false))
        {
            if(timeRaw < 150)
            {
                ((TimingEditor*)WindowManager::GetWindow("Timing"))->SetLatencyOffset(timeRaw);
            }
            else
            {
                ((TimingEditor*)WindowManager::GetWindow("Timing"))->SetLatencyOffset(timeRaw - 200);
            }
        }

        ImGui::EndPopup();
    }
    else if(myLatencyPopupOpenLastFrame)
    {
        stop_latency_metronome();
        ((TimingEditor*)WindowManager::GetWindow("Timing"))->SetInputUnsafe(false);
    }
    // Latency compensation
    DrawLatencyWidget();
    if(ImGui::Button("Open Auto Latency"))
    {
        play_latency_metronome();
        myLatencyStartTime = VAR_FROM_JS(get_audio_context_time()).as<float>();
        ImGui::OpenPopup("Auto Latency");
        myLatencyPopup = true;
        myLatencyPopupOpenLastFrame = true;
        ((TimingEditor*)WindowManager::GetWindow("Timing"))->SetInputUnsafe(true);
    }
    ImGui::SeparatorText("Audio Processor");
    if(ImGui::RadioButton("Default", AudioPlayback::GetEngine() == AudioPlayback::Default))
    {
        AudioPlayback::SetEngine(AudioPlayback::Default);
    }
    ImGui::Indent();
    ImGui::TextDisabled("Fast and consistent on all devices. (Recomended)");
    ImGui::Unindent();
    if(ImGui::RadioButton("RubberBand", AudioPlayback::GetEngine() == AudioPlayback::RubberBand))
    {
        AudioPlayback::SetEngine(AudioPlayback::RubberBand);
    }
    ImGui::Indent();
    ImGui::TextDisabled("The same processor used in Hibikase. \nIt's very slow and won't complete on low power devices.");
    ImGui::Unindent();
    if(ImGui::RadioButton("Browser", AudioPlayback::GetEngine() == AudioPlayback::Browser))
    {
        AudioPlayback::SetEngine(AudioPlayback::Browser);
    }
    ImGui::Indent();
    ImGui::TextDisabled("Uses the browsers own functions. \nVery fast, but some browsers won't do a very good job.");
    ImGui::Unindent();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
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

int Settings::DrawLatencyWidget()
{
    ImGui::Text("Latency (cs)");
    ImGui::SameLine();
    TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
    int latency = timing->GetLatencyOffset();
    if(ImGui::Button("<<", {DPI_SCALED(40), 0}))
    {
        latency -= 5;
    }
    ImGui::SameLine();
    if(ImGui::Button("<", {DPI_SCALED(40), 0}))
    {
        latency -= 1;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(DPI_SCALED(60));
    ImGui::DragInt("##LatencyOffset", &latency);
    ImGui::SameLine();
    if(ImGui::Button(">", {DPI_SCALED(40), 0}))
    {
        latency += 1;
    }
    ImGui::SameLine();
    if(ImGui::Button(">>", {DPI_SCALED(40), 0}))
    {
        latency += 5;
    }
    if(latency != timing->GetLatencyOffset())
    {
        timing->SetLatencyOffset(latency);
    }
    return latency;
}
