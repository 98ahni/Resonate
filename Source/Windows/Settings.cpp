//  This file is licenced under the GNU General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Settings.h"
#include <emscripten.h>
#include "TimingEditor.h"
#include <Extensions/FileHandler.h>
#include <Extensions/imguiExt.h>

extern "C" EMSCRIPTEN_KEEPALIVE void LoadPreferences()
{
    FileHandler::OpenDocument("/local/", ".Resonate");
}
extern "C" EMSCRIPTEN_KEEPALIVE void SavePreferences()
{
    FileHandler::DownloadDocument("/local/.Resonate");
}
extern "C" EMSCRIPTEN_KEEPALIVE void LoadLayout()
{
    FileHandler::OpenDocument(".", ".ini");         // Needs changing as it's currently just a preloaded file in /Assets/
}
extern "C" EMSCRIPTEN_KEEPALIVE void SaveLayout()
{
    FileHandler::DownloadDocument("/imgui.ini");    // Needs changing as it's currently just a preloaded file in /Assets/
}

void Settings::OnImGuiDraw()
{
    Gui_Begin();
    // Latency compensation
    ImGui::Text("Latency (cs)");
    ImGui::SameLine();
    TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
    int latency = timing->GetLatencyOffset();
    if(ImGui::Button("<<"))
    {
        latency -= 5;
    }
    ImGui::SameLine();
    if(ImGui::Button("<"))
    {
        latency -= 1;
    }
    ImGui::SameLine();
    ImGui::DragInt("##LatencyOffset", &latency);
    ImGui::SameLine();
    if(ImGui::Button(">"))
    {
        latency += 1;
    }
    ImGui::SameLine();
    if(ImGui::Button(">>"))
    {
        latency += 5;
    }
    if(latency != timing->GetLatencyOffset())
    {
        timing->SetLatencyOffset(latency);
    }
    // Toggle fullscreen (might not be nessesary)
    // Download/Upload preferences
    ImGui::Button("Export Preferences");
    ImGui::Ext::CreateHTMLButton("ExportPreferences", "click", "_SavePreferences");
    ImGui::SameLine();
    ImGui::Button("Import Preferences");
    ImGui::Ext::CreateHTMLButton("ImportPreferences", "click", "_LoadPreferences");
    // Download/Upload ImGui layout
    ImGui::Button("Export Layout");
    ImGui::Ext::CreateHTMLButton("ExportLayout", "click", "_SaveLayout");
    ImGui::SameLine();
    ImGui::Button("Import Layout");
    ImGui::Ext::CreateHTMLButton("ImportLayout", "click", "_LoadLayout");
    if(!IsOpen())
    {
        ImGui::Ext::DestroyHTMLElement("ExportPreferences", 100);
        ImGui::Ext::DestroyHTMLElement("ImportPreferences", 100);
        ImGui::Ext::DestroyHTMLElement("ExportLayout", 100);
        ImGui::Ext::DestroyHTMLElement("ImportLayout", 100);
    }
    // Clear data
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Button, 0xFFFF0F2F);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0xFFFF1F5F);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xFFFF0F4F);
    if(ImGui::Button("DELETE SAVED DATA"))
    {
        FileHandler::ClearLocalStorage();
        EM_ASM({
            const dbname = '/local/';
            var req = indexedDB.deleteDatabase(dbName);
            req.onsuccess = function() { console.log('Deleted IndexedDB cache ' + dbName + '!'); location.reload();};
            req.onerror = function() { console.error('Failed to delete IndexedDB cache ' + dbName + '!');};
            req.onblocked = function() { console.error('Failed to delete IndexedDB cache ' + dbName + ', DB was blocked!');};
        });
    }
    ImGui::PopStyleColor(3);
    Gui_End();
}

