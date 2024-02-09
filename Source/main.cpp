#include <stdio.h>
#include <emscripten.h>
#include "Windows/MainWindow.h"
#include "Windows/Base/WindowManager.h"
#include "Windows/TimingEditor.h"
#include <GLFW/glfw3.h>
#include <webgl/webgl2.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "Extensions/TouchInput.h"
#include "Extensions/imguiExt.h"
#include "Extensions/FileHandler.h"
#include "Serialization/KaraokeData.h"

bool g_showInputDebugger = false;
char* g_testStr = new char[50];
extern "C" EMSCRIPTEN_KEEPALIVE void ShowInputDebugger() { g_showInputDebugger = true; }
EM_JS(void, show_input_debugger, (), {_ShowInputDebugger(); });

void loop(void* window){
    MainWindow_NewFrame(window);

    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Open Project"))
            {
            }
            ImGui::Ext::CreateHTMLInput("OpenProject", "file", "change", []()
            {
                Serialization::KaraokeDocument::Get().Load(FileHandler::OpenFolder());
            });
            ImGui::EndMenu();
        }
        else
        {
            ImGui::Ext::DestroyHTMLElement("OpenProject", 100);
        }
        ImGui::EndMainMenuBar();
    }

    if(g_showInputDebugger)
    {
        ImGui::Begin("Input Debugger", &g_showInputDebugger);
        ImGui::Text("Mouse position: %f, %f", ImGui::GetMousePos().x, ImGui::GetMousePos().y);
        if(TouchInput_HasTouch()) ImGui::Text("Using Touch");
        else ImGui::Text("Using Mouse");
        ImGui::InputText("Text Input", g_testStr, 50);
        if(ImGui::IsItemClicked(0) && TouchInput_HasTouch()) TouchInput_ReadyKeyboard(false);
        //char* logs = &get_console_logs();
        //ImGui::Text(logs);
        //free(logs);
        ImGui::End();
    }

    WindowManager::ImGuiDraw();

    MainWindow_RenderFrame();
}

int main(){
    void* _window = nullptr;
    MainWindow_Init("Resonate", &_window);
    MainWindow_StyleVarsShadow();
    MainWindow_StyleColorsShadow();
    //MainWindow::Font = ImGui::GetIO().Fonts->AddFontFromFileTTF("Emscripten/Assets/RobotoMono-Regular.ttf", TouchInput_HasTouch() ? 24.0f : 16.0f);
    //ImGui::GetIO().Fonts->Build();
    //ImGui::GetIO().FontDefault = MainWindow::Font;
    //ImGui::PushFont(roboto);

    WindowManager::Init();
    WindowManager::AddWindow<TimingEditor>("Timing");

    emscripten_set_main_loop_arg(loop, (void*)_window, 0, false);
    return 0;
}