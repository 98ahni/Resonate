#include <stdio.h>
#include <emscripten.h>
#include "Windows/MainWindow.h"
#include "Windows/Base/WindowManager.h"
#include "Windows/TimingEditor.h"
#include "Windows/RawText.h"
#include "Windows/AudioPlayback.h"
#include "Windows/TouchControl.h"
#include "Windows/Settings.h"
#include "Windows/Properties.h"
#include <webgl/webgl2.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "Extensions/TouchInput.h"
#include "Extensions/imguiExt.h"
#include "Extensions/FileHandler.h"
#include "Extensions/GoogleDrive.h"
#include "Serialization/KaraokeData.h"
#include "Serialization/Syllabify.h"
#include "Serialization/Preferences.h"
#include "StringTools.h"
#include "Defines.h"
#include <filesystem>

bool g_showInputDebugger = false;
char* g_testStr = new char[50];
extern "C" EMSCRIPTEN_KEEPALIVE void ShowInputDebugger() { g_showInputDebugger = true; }
EM_JS(void, show_input_debugger, (), {_ShowInputDebugger(); });

bool g_fileTabOpenedThisFrame = true; // Only use in File tab!
bool g_closeFileTab = false;
bool g_hasGoogleAcc = false;

extern "C" EMSCRIPTEN_KEEPALIVE void LoadProject()
{
    //AudioPlayback::PrepPlayback();
    std::string folderPath = FileHandler::OpenFolder();
    if(folderPath == "") return;
    FileHandler::ClearLocalStorage();
    Serialization::KaraokeDocument::Get().Load(folderPath);
    AudioPlayback::SetPlaybackFile(folderPath);
    g_closeFileTab = true;
}
extern "C" EMSCRIPTEN_KEEPALIVE void SaveProject()
{
    std::string docPath = Serialization::KaraokeDocument::Get().Save();
    //std::string docPath = AudioPlayback::GetPath();       // Test audio file
    FileHandler::DownloadDocument(docPath.c_str());
    Serialization::KaraokeDocument::Get().UnsetIsDirty();
    g_closeFileTab = true;
}

extern "C" EMSCRIPTEN_KEEPALIVE void LoadFileFromGoogleDrive(emscripten::EM_VAL aFSPath, emscripten::EM_VAL aFileID)
{
    std::filesystem::path path = VAR_FROM_JS(aFSPath).as<std::string>();
    std::string id = VAR_FROM_JS(aFileID).as<std::string>();
    printf("Loaded project '%s' with file id: %s\n", path.string().data(), id.data());
    if(path.extension() == ".txt")
    {
        Serialization::Preferences::SetString("Document/FileID", id);
        Serialization::KaraokeDocument::Get().Load(path.string(), id);
    }
    else if(path.extension() == ".mp3")
    {
        AudioPlayback::SetPlaybackFile(path.string());
    }
}
extern "C" EMSCRIPTEN_KEEPALIVE void LoadCanceledFromGoogleDrive()
{
    Serialization::KaraokeDocument::Get().AutoSave();
    AudioPlayback::SaveLocalBackup();
}

void loop(void* window){
    MainWindow_NewFrame(window);
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    MainWindow_SetName(doc.GetName().empty() ? "Resonate" : (doc.GetIsDirty() ? "*" : "") + doc.GetName() + " - Resonate");
    MainWindow_SetIcon(doc.GetIsDirty() ? "ResonateIconUnsaved.png" : "ResonateIcon.png");
    if(doc.GetIsAutoDirty())
    {
        doc.AutoSave();
    }

    if(ImGui::BeginMainMenuBar())
    {
        if(!g_closeFileTab && ImGui::BeginMenu("File"))
        {
            if(g_fileTabOpenedThisFrame)
            {
                g_hasGoogleAcc = GoogleDrive::HasToken();
            }
            if(ImGui::MenuItem("Open Project"))
            {
            }
            ImGui::Ext::CreateHTMLButton("OpenProject", "click", "_LoadProject");
            if(ImGui::MenuItem("Save Document"))
            {
            }
            ImGui::Ext::CreateHTMLButton("SaveProject", "click", "_SaveProject");
            ImGui::Separator();
            if(ImGui::BeginMenu("Google Drive"))
            {
                if(!g_hasGoogleAcc)
                {
                    if(ImGui::MenuItem("Log In With Google"))
                    {
                        GoogleDrive::RequestToken(!Serialization::Preferences::HasKey("Google/IsLoggedIn"));
                        Serialization::Preferences::SetBool("Google/IsLoggedIn", true);
                    }
                }
                else
                {
                    // Profile info + logout
                }
                ImGui::Separator();
                if(ImGui::MenuItem("Open Project", 0, false, g_hasGoogleAcc))
                {
                    FileHandler::ClearLocalStorage();
                    GoogleDrive::LoadProject("application/vnd.google-apps.folder", "_LoadFileFromGoogleDrive", "_LoadCanceledFromGoogleDrive");
                }
                if(ImGui::MenuItem("Save Document", 0, false, g_hasGoogleAcc && doc.GetFileID() != ""))
                {
                    GoogleDrive::SaveProject(doc.GetFileID(), doc.Save());
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
            g_fileTabOpenedThisFrame = false;
        }
        else
        {
            g_fileTabOpenedThisFrame = true;
            g_closeFileTab = false;
            ImGui::Ext::DestroyHTMLElement("OpenProject");
            ImGui::Ext::DestroyHTMLElement("SaveProject");
        }
        if(ImGui::BeginMenu("Edit"))
        {
            if(ImGui::MenuItem("Insert Line Break"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                doc.InsertLineBreak(timing->GetMarkedLine(), timing->GetMarkedToken(), timing->GetMarkedChar());
            }
            if(ImGui::MenuItem("Merge Line Up"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                doc.RevoveLineBreak(timing->GetMarkedLine());
            }
            if(ImGui::MenuItem("Merge Line Down"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                doc.RevoveLineBreak(timing->GetMarkedLine() + 1);
            }
            if(ImGui::MenuItem("Move Line Up"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                doc.MoveLineUp(timing->GetMarkedLine());
            }
            if(ImGui::MenuItem("Move Line Down"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                doc.MoveLineUp(timing->GetMarkedLine() + 1);
            }
            if(ImGui::MenuItem("Duplicate Line"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                doc.DuplicateLine(timing->GetMarkedLine());
            }
            ImGui::EndMenu();
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
            if(ImGui::MenuItem("Style Properties", 0, WindowManager::GetWindow("Properties") != nullptr))
            {
                if(WindowManager::GetWindow("Properties") != nullptr)
                {
                    WindowManager::DestroyWindow(WindowManager::GetWindow("Properties"));
                }
                else
                {
                    WindowManager::AddWindow<PropertiesWindow>("Properties");
                }
            }
            if(ImGui::MenuItem("Settings", 0, WindowManager::GetWindow("Settings") != nullptr))
            {
                if(WindowManager::GetWindow("Settings") != nullptr)
                {
                    WindowManager::DestroyWindow(WindowManager::GetWindow("Settings"));
                }
                else
                {
                    WindowManager::AddWindow<Settings>("Settings");
                }
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
                        std::string text = doc.SerializeAsText();
                        printf("Done serializing.\n");
                        std::vector<std::string> tokenList = Serialization::Syllabify(text, code);
                        text = StringTools::Join(tokenList, "[00:00:00]");
                        printf("Done syllabifying.\n");
                        printf("%s\n", text.data());
                        doc.Parse("[00:00:00]" + text + "[00:00:00]");
                        doc.MakeDirty();
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
                        std::vector<std::string> tokenList = Serialization::Syllabify(doc.SerializeLineAsText(doc.GetLine(((TimingEditor*)WindowManager::GetWindow("Timing"))->GetMarkedLine())), code);
                        doc.ParseLineAndReplace(StringTools::Join(tokenList, "[00:00:00]"), ((TimingEditor*)WindowManager::GetWindow("Timing"))->GetMarkedLine());
                        doc.MakeDirty();
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("About"))
        {
            if(ImGui::MenuItem("Help", 0, WindowManager::GetWindow("Touch Control") != nullptr))
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
            if(ImGui::MenuItem("Guidelines"))
            {
                // Open the guidelines in a new window.
            }
            if(ImGui::MenuItem("Licence", 0, WindowManager::GetWindow("Licence") != nullptr))
            {
                if(WindowManager::GetWindow("Licence") != nullptr)
                {
                    WindowManager::DestroyWindow(WindowManager::GetWindow("Licence"));
                }
                else
                {
                    WindowManager::AddWindow<TouchControl>("Licence");
                }
            }
            if(ImGui::MenuItem("Report a Bug"))
            {
                // Open the Resonate GitHub Issues page.
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

    Serialization::Syllabify_Init();
    Serialization::LoadPrefs();
    Serialization::KaraokeDocument::Get().Load("/local", (
        Serialization::Preferences::HasKey("Document/FileID") ? Serialization::Preferences::GetString("Document/FileID") : ""
    ));

    WindowManager::Init();
    TimingEditor* timingEditor = WindowManager::AddWindow<TimingEditor>("Timing");
    WindowManager::AddWindow<AudioPlayback>("Audio");
    WindowManager::AddWindow<TextEditor>("Raw Text");

    ImGui::GetIO().Fonts->AddFontDefault(nullptr);
    // Vv For the video previewer. vV
    //videoPreview->SetFont(ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/FredokaOne-Regular.ttf", TouchInput_HasTouch() ? 24.0f : 20.0f));
    timingEditor->SetFont(ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Fredoka-Regular.ttf", TouchInput_HasTouch() ? 20.0f : 20.0f));
    ImGui::GetIO().Fonts->Build();
    //ImGui::GetIO().FontDefault = MainWindow::Font;
    //ImGui::PushFont(roboto);

    emscripten_set_main_loop_arg(loop, (void*)_window, 0, false);
    return 0;
}