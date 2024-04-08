#include <stdio.h>
#include <emscripten.h>
#include "Windows/MainWindow.h"
#include "Windows/Base/WindowManager.h"
#include "Windows/TimingEditor.h"
#include "Windows/RawText.h"
#include "Windows/AudioPlayback.h"
#include "Windows/TouchControl.h"
#include <GLFW/glfw3.h>
#include <webgl/webgl2.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "Extensions/TouchInput.h"
#include "Extensions/imguiExt.h"
#include "Extensions/FileHandler.h"
#include "Serialization/KaraokeData.h"
#include "Serialization/Syllabify.h"
#include "StringTools.h"

bool g_showInputDebugger = false;
char* g_testStr = new char[50];
extern "C" EMSCRIPTEN_KEEPALIVE void ShowInputDebugger() { g_showInputDebugger = true; }
EM_JS(void, show_input_debugger, (), {_ShowInputDebugger(); });

bool g_closeFileTab = false;
extern "C" EMSCRIPTEN_KEEPALIVE void LoadProject()
{
    //AudioPlayback::PrepPlayback();
    std::string folderPath = FileHandler::OpenFolder();
    Serialization::KaraokeDocument::Get().Load(folderPath);
    AudioPlayback::SetPlaybackFile(folderPath);
    g_closeFileTab = true;
}
extern "C" EMSCRIPTEN_KEEPALIVE void SaveProject()
{
    std::string docPath = Serialization::KaraokeDocument::Get().Save();
    FileHandler::DownloadDocument(docPath.c_str());
    g_closeFileTab = true;
}

void loop(void* window){
    MainWindow_NewFrame(window);

    if(ImGui::BeginMainMenuBar())
    {
        if(!g_closeFileTab && ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Open Project"))
            {
            }
            ImGui::Ext::CreateHTMLButton("OpenProject", "touchend", "_LoadProject");
            if(ImGui::MenuItem("Save Document"))
            {
            }
            ImGui::Ext::CreateHTMLButton("SaveProject", "click", "_SaveProject");
            ImGui::EndMenu();
        }
        else
        {
            g_closeFileTab = false;
            ImGui::Ext::DestroyHTMLElement("OpenProject");
            ImGui::Ext::DestroyHTMLElement("SaveProject");
        }
        if(ImGui::BeginMenu("View"))
        {
            if(ImGui::MenuItem("Touch Control", 0, WindowManager::GetWindow("Touch Control") != nullptr))
            {
                if(WindowManager::GetWindow("Touch Control") != nullptr)
                {
                    WindowManager::DestroyWindow(WindowManager::GetWindow("Touch Control"));
                }
                else
                {
                    WindowManager::AddWindow<TouchControl>("Touch Control");
                }
            }
            if(ImGui::MenuItem("Print ini"))
            {
                printf("%s\n", ImGui::SaveIniSettingsToMemory());
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Syllabify"))
        {
            if(ImGui::BeginMenu("All"))
            {
                for(auto&[code, name] : Serialization::GetAvailableLanguages())
                {
                    if(ImGui::MenuItem(name.c_str()))
                    {
                        Serialization::BuildPatterns(code);
                        std::string text = Serialization::KaraokeDocument::Get().SerializeAsText();
                        printf("Done serializing.\n");
                        std::vector<std::string> tokenList = Serialization::Syllabify(text, code);
                        text = StringTools::Join(tokenList, "[00:00:00]");
                        printf("Done syllabifying.\n");
                        printf("%s\n", text.data());
                        Serialization::KaraokeDocument::Get().Parse("[00:00:00]" + text + "[00:00:00]");
                    }
                }
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("Line"))
            {
                for(auto&[code, name] : Serialization::GetAvailableLanguages())
                {
                    if(ImGui::MenuItem(name.c_str()))
                    {
                        Serialization::BuildPatterns(code);
                        std::vector<std::string> tokenList = Serialization::Syllabify(Serialization::KaraokeDocument::Get().SerializeLineAsText(Serialization::KaraokeDocument::Get().GetLine(((TimingEditor*)WindowManager::GetWindow("Timing"))->GetMarkedLine())), code);
                        Serialization::KaraokeDocument::Get().ParseLineAndReplace(StringTools::Join(tokenList, "[00:00:00]"), ((TimingEditor*)WindowManager::GetWindow("Timing"))->GetMarkedLine());
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
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

    Serialization::Syllabify_Init();

    WindowManager::Init();
    WindowManager::AddWindow<TimingEditor>("Timing");
    WindowManager::AddWindow<AudioPlayback>("Audio");
    WindowManager::AddWindow<TextEditor>("Raw Text");

    emscripten_set_main_loop_arg(loop, (void*)_window, 0, false);
    return 0;
}