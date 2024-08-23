//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Resonate. Copyright (C) 2024 98ahni and Resonate Autohrs>
//  <Copyright (C) 2024 98ahni> Original file author

#include <stdio.h>
#include <emscripten.h>
#include <emscripten/val.h>
#include "Windows/MainWindow.h"
#include "Windows/Base/WindowManager.h"
#include "Windows/TimingEditor.h"
#include "Windows/RawText.h"
#include "Windows/AudioPlayback.h"
#include "Windows/TouchControl.h"
#include "Windows/Settings.h"
#include "Windows/Properties.h"
#include "Windows/Help.h"
#include "Windows/License.h"
#include "Windows/Preview.h"
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

bool g_hasGoogleAcc = false;
bool g_fileTabOpenedThisFrame = true; // Only use in File tab!
bool g_closeFileTab = false;
bool g_closeAboutTab = false;

extern "C" EMSCRIPTEN_KEEPALIVE void LoadProject()
{
    //AudioPlayback::PrepPlayback();
    std::string folderPath = FileHandler::OpenFolder();
    if(folderPath == "") return;
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

extern "C" EMSCRIPTEN_KEEPALIVE void GoogleTokenExpirationCallback(emscripten::EM_VAL aTime)
{
    Serialization::Preferences::SetDouble("Google/ExpirationDate", VAR_FROM_JS(aTime).as<double>());
    Serialization::Preferences::SetBool("Google/IsLoggedIn", true);
}
extern "C" EMSCRIPTEN_KEEPALIVE void LogInToGoogle()
{
    float expiration = Serialization::Preferences::HasKey("Google/ExpirationDate") ? Serialization::Preferences::GetDouble("Google/ExpirationDate") : 0;
    GoogleDrive::RequestToken(EM_ASM_DOUBLE({return Date.now();}) >= expiration, "GoogleTokenExpirationCallback");
    //GoogleDrive::RequestToken(!Serialization::Preferences::HasKey("Google/IsLoggedIn"));
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

EM_JS(void, open_mooncat_guidelines, (), {
    window.open('https://docs.google.com/document/d/1pNXmutbveAyj_UmFDs7y2M3-1R6-rFECsc_SPUnWSDQ/edit?usp=sharing', '_blank');
});
EM_JS(void, open_resonate_issues, (), {
    window.open('https://github.com/98ahni/Resonate/issues', '_blank');
});

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
            ImGui::MenuItem("Open Project");
            ImGui::Ext::CreateHTMLButton("OpenProject", "click", "_LoadProject");
            ImGui::MenuItem("Save Document");
            ImGui::Ext::CreateHTMLButton("SaveProject", "click", "_SaveProject");
            ImGui::Separator();
            if(ImGui::BeginMenu("Google Drive", GoogleDrive::Ready()))
            {
                if(!g_hasGoogleAcc)
                {
                    ImGui::MenuItem("Log In With Google");
                    ImGui::Ext::CreateHTMLButton("GoogleLogin", "click", "_LogInToGoogle");
                }
                else
                {
                    // Profile info + logout
                }
                ImGui::Separator();
                if(ImGui::MenuItem("Open Project", 0, false, g_hasGoogleAcc))
                {
                    GoogleDrive::LoadProject("application/vnd.google-apps.folder", "_LoadFileFromGoogleDrive", "_LoadCanceledFromGoogleDrive");
                }
                if(ImGui::MenuItem("Save Document", 0, false, g_hasGoogleAcc && doc.GetFileID() != ""))
                {
                    GoogleDrive::SaveProject(doc.GetFileID(), doc.Save());
                    Serialization::KaraokeDocument::Get().UnsetIsDirty();
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
            ImGui::Ext::DestroyHTMLElement("GoogleLogin");
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
            ImGui::Separator();
            if(ImGui::MenuItem("Remove Line"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                doc.RemoveLine(timing->GetMarkedLine());
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Effects"))
        {
            if(ImGui::MenuItem("Preview", 0, WindowManager::GetWindow("Preview") != nullptr))
            {
                if(WindowManager::GetWindow("Preview") != nullptr)
                {
                    WindowManager::DestroyWindow(WindowManager::GetWindow("Preview"));
                }
                else
                {
                    WindowManager::AddWindow<PreviewWindow>("Preview");
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
            ImGui::BeginDisabled();
            ImGui::SeparatorText("Line Effects");
            ImGui::EndDisabled();
            if(ImGui::BeginMenu("Image", false))
            {
                // This menu should be active if there are any images in the project.
                ImGui::EndMenu();
            }
            if(ImGui::MenuItem("No Effect", "<no effect>"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                doc.GetLine(timing->GetMarkedLine()).insert(doc.GetLine(timing->GetMarkedLine()).begin(), {"<no effect>", false, 0});
            }
            if(ImGui::MenuItem("Display Line", "<line#>", nullptr, false))
            {
                // This should display buttons in the Timing Editor for changing the value.
            }
            ImGui::BeginDisabled();
            ImGui::SeparatorText("Text Effects");
            ImGui::EndDisabled();
            for(const auto& [alias, effect] : doc.GetEffectAliases())
            {
                if(ImGui::MenuItem(alias.data(), effect->myECHOValue.data()))
                {
                    TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                    Serialization::KaraokeToken& token = doc.GetToken(timing->GetMarkedLine(), timing->GetMarkedToken());
                    doc.GetLine(timing->GetMarkedLine()).insert(doc.GetLine(timing->GetMarkedLine()).begin() + timing->GetMarkedToken(), {("<" + alias + ">").data(), true, token.myStartTime});
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
            if(ImGui::MenuItem("Help", 0, WindowManager::GetWindow("Help") != nullptr))
            {
                if(WindowManager::GetWindow("Help") != nullptr)
                {
                    WindowManager::DestroyWindow(WindowManager::GetWindow("Help"));
                }
                else
                {
                    WindowManager::AddWindow<HelpWindow>("Help");
                }
            }
            ImGui::MenuItem("Guidelines");
            ImGui::Ext::CreateHTMLButton("MooncatGuidelines", "click", "open_mooncat_guidelines");
            if(ImGui::MenuItem("Licence", 0, WindowManager::GetWindow("Licence") != nullptr))
            {
                if(WindowManager::GetWindow("Licence") != nullptr)
                {
                    WindowManager::DestroyWindow(WindowManager::GetWindow("Licence"));
                }
                else
                {
                    WindowManager::AddWindow<LicenseWindow>("Licence");
                }
            }
            ImGui::MenuItem("Report a Bug");
            ImGui::Ext::CreateHTMLButton("RepportBug", "click", "open_resonate_issues");
#if _DEBUG
            if(ImGui::MenuItem("Print Prefs"))
            {
                Serialization::PrintPrefs();
            }
            if(ImGui::MenuItem("Reload Page"))
            {
                EM_ASM(location.reload());
            }
#endif
            ImGui::EndMenu();
        }
        else
        {
            g_closeAboutTab = false;
            ImGui::Ext::DestroyHTMLElement("MooncatGuidelines");
            ImGui::Ext::DestroyHTMLElement("RepportBug");
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
#if defined(GOOGLE_API_SECRET)
    printf("Secrets work\n");
#endif
    Serialization::Syllabify_Init();
    Serialization::LoadPrefs();
    Serialization::KaraokeDocument::Get().Load("/local", (
        Serialization::Preferences::HasKey("Document/FileID") ? Serialization::Preferences::GetString("Document/FileID") : ""
    ));

    ImGui::Ext::SetShortcutEvents();
    
    WindowManager::Init();
    TimingEditor* timingEditor = WindowManager::AddWindow<TimingEditor>("Timing");
    WindowManager::AddWindow<AudioPlayback>("Audio");
    WindowManager::AddWindow<TextEditor>("Raw Text");
    ImGui::SetWindowFocus("Timing");

    ImGui::GetIO().Fonts->AddFontDefault(nullptr);
    // Vv For the video previewer. vV
    //videoPreview->SetFont(ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/FredokaOne-Regular.ttf", TouchInput_HasTouch() ? 24.0f : 20.0f));
    ImFont* timingFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Fredoka-Regular.ttf", TouchInput_HasTouch() ? 40.0f : 40.0f);
    timingEditor->SetFont(timingFont);
    timingFont->Scale = .5f;
    MainWindow::Font = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Fredoka-Regular.ttf", TouchInput_HasTouch() ? 40.0f : 40.0f);
    MainWindow::Font->Scale = .5f;
    ImGui::GetIO().Fonts->Build();
    //ImGui::PushFont(roboto);

    emscripten_set_main_loop_arg(loop, (void*)_window, 0, false);
    return 0;
}