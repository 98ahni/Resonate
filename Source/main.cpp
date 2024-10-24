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
#include "Extensions/Dropbox.h"
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
bool g_hasDropboxAcc = false;
bool g_fileTabOpenedThisFrame = true; // Only use in File tab!
bool g_closeFileTab = false;
bool g_closeAboutTab = false;
bool g_shouldDeleteOnLoad = false;
bool g_firstFrameAfterFileLoad = false;

bool g_shouldHideLoadingScreen = false;

void DrawSelfTestWarningPopup();
bool g_selfTestInProgress = true;
bool g_selfTestFailed = false;
bool g_isSafeMode = false;

extern "C" EMSCRIPTEN_KEEPALIVE void LoadProject()
{
    ImGui::Ext::StartLoadingScreen();
    //AudioPlayback::PrepPlayback();
    std::string folderPath = FileHandler::OpenFolder();
    if(folderPath == "") return;
    Serialization::KaraokeDocument::Get().Load(folderPath);
    AudioPlayback::SetPlaybackFile(folderPath);
    PreviewWindow::ClearBackgroundElements();
    PreviewWindow::AddBackgroundElement(folderPath);
    FileHandler::SyncLocalFS();
    g_closeFileTab = true;
    ImGui::Ext::StopLoadingScreen();
}
extern "C" EMSCRIPTEN_KEEPALIVE void SaveProject()
{
    ImGui::Ext::StartLoadingScreen();
    std::string docPath = Serialization::KaraokeDocument::Get().Save();
    //std::string docPath = AudioPlayback::GetPath();       // Test audio file
    FileHandler::DownloadDocument(docPath.c_str());
    Serialization::KaraokeDocument::Get().UnsetIsDirty();
    FileHandler::SyncLocalFS();
    g_closeFileTab = true;
    ImGui::Ext::StopLoadingScreen();
}

extern "C" EMSCRIPTEN_KEEPALIVE void GoogleTokenExpirationCallback(emscripten::EM_VAL aTime)
{
    Serialization::Preferences::SetDouble("Google/ExpirationDate", VAR_FROM_JS(aTime).as<double>());
    Serialization::Preferences::SetBool("Google/IsLoggedIn", true);
}
extern "C" EMSCRIPTEN_KEEPALIVE void LogInToGoogle()
{
    float expiration = Serialization::Preferences::HasKey("Google/ExpirationDate") ? Serialization::Preferences::GetDouble("Google/ExpirationDate") : 0;
    GoogleDrive::RequestToken(EM_ASM_DOUBLE({return Date.now();}) >= expiration, "_GoogleTokenExpirationCallback");
    //GoogleDrive::RequestToken(!Serialization::Preferences::HasKey("Google/IsLoggedIn"));
    g_closeFileTab = true;
}

extern "C" EMSCRIPTEN_KEEPALIVE void DropboxTokenExpirationCallback(emscripten::EM_VAL aTime)
{
    Serialization::Preferences::SetDouble("Dropbox/ExpirationDate", VAR_FROM_JS(aTime).as<double>());
    Serialization::Preferences::SetBool("Dropbox/IsLoggedIn", true);
}
extern "C" EMSCRIPTEN_KEEPALIVE void LogInToDropbox()
{
    float expiration = Serialization::Preferences::HasKey("Dropbox/ExpirationDate") ? Serialization::Preferences::GetDouble("Dropbox/ExpirationDate") : 0;
    Dropbox::RequestToken(EM_ASM_DOUBLE({return Date.now();}) >= expiration, "_DropboxTokenExpirationCallback");
    //Dropbox::RequestToken(!Serialization::Preferences::HasKey("Dropbox/IsLoggedIn"));
    g_closeFileTab = true;
}
extern "C" EMSCRIPTEN_KEEPALIVE void OpenDropboxChooser()
{
    g_shouldDeleteOnLoad = true;
    ImGui::Ext::StartLoadingScreen();
    Dropbox::LoadProject("_LoadFileFromCloudDrive", "_LoadCompletedFromCloudDrive", "_LoadCanceledFromCloudDrive");
}

extern "C" EMSCRIPTEN_KEEPALIVE void LoadFileFromCloudDrive(emscripten::EM_VAL aFSPath, emscripten::EM_VAL aFileID)
{
    if(g_shouldDeleteOnLoad)
    {
        PreviewWindow::ClearBackgroundElements();
        g_shouldDeleteOnLoad = false;
    }
    std::filesystem::path path = VAR_FROM_JS(aFSPath).as<std::string>();
    std::string id = VAR_FROM_JS(aFileID).as<std::string>();
    printf("Loaded project '%s' with file id: %s\n", path.string().data(), id.data());
    if(path.extension() == ".txt")
    {
        Serialization::KaraokeDocument::Get().Load(path.string(), id, false);
    }
    else if(path.extension() == ".mp3")
    {
        AudioPlayback::SetPlaybackFile(path.string());
    }
    else if(path.extension() == ".mp4" || path.extension() == ".png" || path.extension() == ".jpg")
    {
        PreviewWindow::AddBackgroundElement(path.string());
    }
    //FileHandler::SyncLocalFS();
}
extern "C" EMSCRIPTEN_KEEPALIVE void LoadCompletedFromCloudDrive()
{
    g_firstFrameAfterFileLoad = true;
    g_shouldHideLoadingScreen = true;
}
extern "C" EMSCRIPTEN_KEEPALIVE void LoadCanceledFromCloudDrive()
{
    g_shouldDeleteOnLoad = false;
    g_shouldHideLoadingScreen = true;
    Serialization::KaraokeDocument::Get().AutoSave();
    AudioPlayback::SaveLocalBackup();
    FileHandler::SyncLocalFS();
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
    DrawSelfTestWarningPopup();
    if(doc.GetIsAutoDirty())
    {
        doc.AutoSave();
        FileHandler::SyncLocalFS();
    }
    if(g_firstFrameAfterFileLoad)
    {
        g_firstFrameAfterFileLoad = false;
        Serialization::Preferences::SetString("Document/FileID", doc.GetFileID());
        doc.ParseLoadedFile();
        FileHandler::SyncLocalFS();
    }

    if(ImGui::BeginMainMenuBar())
    {
        if(!g_closeFileTab && ImGui::BeginMenu("File"))
        {
            if(g_fileTabOpenedThisFrame)
            {
                g_hasGoogleAcc = GoogleDrive::HasToken() && EM_ASM_DOUBLE({return Date.now();}) >= Serialization::Preferences::GetDouble("Google/ExpirationDate");
                g_hasDropboxAcc = Dropbox::HasToken() && 
                    //(!Serialization::Preferences::HasKey("Dropbox/ExpirationDate") ||
                    EM_ASM_DOUBLE({return Date.now();}) >= Serialization::Preferences::GetDouble("Dropbox/ExpirationDate");
            }
            ImGui::MenuItem("Open Project", 0, false, !g_isSafeMode);
            if(!g_isSafeMode){ImGui::Ext::CreateHTMLButton("OpenProject", "click", "_LoadProject");}
            ImGui::MenuItem("Save Document");
            ImGui::Ext::CreateHTMLButton("SaveProject", "click", "_SaveProject");
            ImGui::Separator();
            if(ImGui::BeginMenu("Google Drive", GoogleDrive::Ready()))
            {
                if(!g_hasGoogleAcc)
                {
                    if(ImGui::MenuItem("Log In With Google"))
                    {
                        //LogInToGoogle();
                    }
                    ImGui::Ext::CreateHTMLButton("GoogleLogin", "click", "_LogInToGoogle");
                }
                else
                {
                    // Profile info + logout
                }
                ImGui::Separator();
                if(ImGui::MenuItem("Open Project", 0, false, !g_isSafeMode && g_hasGoogleAcc))
                {
                    g_shouldDeleteOnLoad = true;
                    ImGui::Ext::StartLoadingScreen();
                    GoogleDrive::LoadProject("application/vnd.google-apps.folder", "_LoadFileFromCloudDrive", "_LoadCompletedFromCloudDrive", "_LoadCanceledFromCloudDrive");
                }
                if(ImGui::MenuItem("Save Document", 0, false, g_hasGoogleAcc && doc.GetFileID() != ""))
                {
                    ImGui::Ext::StartLoadingScreen();
                    GoogleDrive::SaveProject(doc.GetFileID(), doc.Save());
                    Serialization::KaraokeDocument::Get().UnsetIsDirty();
                    FileHandler::SyncLocalFS();
                    ImGui::Ext::StopLoadingScreen();
                }
                ImGui::EndMenu();
            }
            else
            {
                ImGui::Ext::DestroyHTMLElement("GoogleLogin");
            }
            if(ImGui::BeginMenu("Dropbox"))
            {
                if(!g_hasDropboxAcc)
                {
                    if(ImGui::MenuItem("Log In to Dropbox"))
                    {
                        //LogInToDropbox();
                    }
                    ImGui::Ext::CreateHTMLButton("DropboxLogin", "click", "_LogInToDropbox");
                }
                else
                {
                    // Profile info + logout
                }
                ImGui::Separator();
                ImGui::MenuItem("Open Project", 0, false, !g_isSafeMode && g_hasDropboxAcc);
                if(!g_isSafeMode && g_hasDropboxAcc) {ImGui::Ext::CreateHTMLButton("OpenDBProject", "click", "_OpenDropboxChooser");}
                if(ImGui::MenuItem("Save Document", 0, false, g_hasDropboxAcc && doc.GetFileID() != ""))
                {
                    ImGui::Ext::StartLoadingScreen();
                    Dropbox::SaveProject(doc.GetFileID(), doc.Save());
                    Serialization::KaraokeDocument::Get().UnsetIsDirty();
                    FileHandler::SyncLocalFS();
                    ImGui::Ext::StopLoadingScreen();
                }
                ImGui::EndMenu();
            }
            else
            {
                ImGui::Ext::DestroyHTMLElement("DropboxLogin");
                ImGui::Ext::DestroyHTMLElement("OpenDBProject");
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
            ImGui::Ext::DestroyHTMLElement("DropboxLogin");
            ImGui::Ext::DestroyHTMLElement("OpenDBProject");
        }
        if(ImGui::BeginMenu("Edit", !g_isSafeMode))
        {
            if(ImGui::MenuItem("Insert Line Break"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                doc.InsertLineBreak(timing->GetMarkedLine(), timing->GetMarkedToken(), timing->GetMarkedChar());
            }
            if(ImGui::MenuItem("Merge Line Up"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                if(!(doc.GetLine(timing->GetMarkedLine() - 1).size() == 1 && (doc.GetToken(timing->GetMarkedLine() - 1, 0).myValue.starts_with("image") || (doc.GetLine(timing->GetMarkedLine() - 2).size() == 1 && doc.GetToken(timing->GetMarkedLine() - 2, 0).myValue.starts_with("image")))))
                {
                    doc.RevoveLineBreak(timing->GetMarkedLine());
                }
            }
            if(ImGui::MenuItem("Merge Line Down"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                if(!(doc.GetLine(timing->GetMarkedLine() + 1).size() == 1 && doc.GetToken(timing->GetMarkedLine() + 1, 0).myValue.starts_with("image")))
                {
                    doc.RevoveLineBreak(timing->GetMarkedLine() + 1);
                }
            }
            if(ImGui::MenuItem("Move Line Up"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                doc.MoveLineUp(timing->GetMarkedLine());
                if(timing->GetMarkedLine() > 1 && doc.GetLine(timing->GetMarkedLine() - 2).size() == 1 && doc.GetToken(timing->GetMarkedLine() - 2, 0).myValue.starts_with("image"))
                {
                    doc.MoveLineUp(timing->GetMarkedLine() - 1);
                }
            }
            if(ImGui::MenuItem("Move Line Down"))
            {
                TimingEditor* timing = (TimingEditor*)WindowManager::GetWindow("Timing");
                doc.MoveLineUp(timing->GetMarkedLine() + 1);
                if(doc.GetLine(timing->GetMarkedLine() + 2).size() == 1 && doc.GetLine(timing->GetMarkedLine()).size() == 1 && doc.GetToken(timing->GetMarkedLine(), 0).myValue.starts_with("image"))
                {
                    doc.MoveLineUp(timing->GetMarkedLine() + 2);
                }
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
            ImGui::Separator();
            ImGui::SeparatorText("Word Case");
            if(ImGui::MenuItem("Majiscule", "EX,AM,PLE"))
            {
                int markedLine = TimingEditor::Get().GetMarkedLine();
                // Find first token of word
                int leadingSpaceInd = TimingEditor::Get().GetMarkedToken() - 1;
                while(leadingSpaceInd >= 0 && !doc.GetToken(markedLine, leadingSpaceInd).myValue.contains(' '))
                {
                    leadingSpaceInd--;
                }
                // Find last token of word
                int trailingSpaceInd = TimingEditor::Get().GetMarkedToken();
                while(trailingSpaceInd < doc.GetLine(markedLine).size() && !doc.GetToken(markedLine, trailingSpaceInd).myValue.contains(' '))
                {
                    trailingSpaceInd++;
                }
                // Replace letters
                if(leadingSpaceInd >= 0) for(int i = doc.GetToken(markedLine, leadingSpaceInd).myValue.find_last_of(' '); i < doc.GetToken(markedLine, leadingSpaceInd).myValue.size(); i++)
                {
                    doc.GetToken(markedLine, leadingSpaceInd).myValue[i] = std::toupper(doc.GetToken(markedLine, leadingSpaceInd).myValue[i]);
                }
                for(int i = leadingSpaceInd + 1; i < trailingSpaceInd; i++)
                {
                    for(int j = 0; j < doc.GetToken(markedLine, i).myValue.size(); j++)
                    {
                        doc.GetToken(markedLine, i).myValue[j] = std::toupper(doc.GetToken(markedLine, i).myValue[j]);
                    }
                }
                for(int i = 0; i < doc.GetToken(markedLine, trailingSpaceInd).myValue.size(); i++)
                {
                    if(doc.GetToken(markedLine, trailingSpaceInd).myValue[i] == ' ') { break; }
                    doc.GetToken(markedLine, trailingSpaceInd).myValue[i] = std::toupper(doc.GetToken(markedLine, trailingSpaceInd).myValue[i]);
                }
            }
            if(ImGui::MenuItem("Minuscule", "ex,am,ple"))
            {
                int markedLine = TimingEditor::Get().GetMarkedLine();
                // Find first token of word
                int leadingSpaceInd = TimingEditor::Get().GetMarkedToken();
                while(leadingSpaceInd >= 0 && !doc.GetToken(markedLine, leadingSpaceInd).myValue.contains(' '))
                {
                    leadingSpaceInd--;
                }
                // Find last token of word
                int trailingSpaceInd = TimingEditor::Get().GetMarkedToken();
                while(trailingSpaceInd < doc.GetLine(markedLine).size() && !doc.GetToken(markedLine, trailingSpaceInd).myValue.contains(' '))
                {
                    trailingSpaceInd++;
                }
                // Replace letters
                if(leadingSpaceInd >= 0) for(int i = doc.GetToken(markedLine, leadingSpaceInd).myValue.find_last_of(' '); i < doc.GetToken(markedLine, leadingSpaceInd).myValue.size(); i++)
                {
                    doc.GetToken(markedLine, leadingSpaceInd).myValue[i] = std::tolower(doc.GetToken(markedLine, leadingSpaceInd).myValue[i]);
                }
                for(int i = leadingSpaceInd + 1; i < trailingSpaceInd; i++)
                {
                    for(int j = 0; j < doc.GetToken(markedLine, i).myValue.size(); j++)
                    {
                        doc.GetToken(markedLine, i).myValue[j] = std::tolower(doc.GetToken(markedLine, i).myValue[j]);
                    }
                }
                for(int i = 0; i < doc.GetToken(markedLine, trailingSpaceInd).myValue.size(); i++)
                {
                    if(doc.GetToken(markedLine, trailingSpaceInd).myValue[i] == ' ') { break; }
                    doc.GetToken(markedLine, trailingSpaceInd).myValue[i] = std::tolower(doc.GetToken(markedLine, trailingSpaceInd).myValue[i]);
                }
            }
            if(ImGui::MenuItem("Capital", "Ex,am,ple"))
            {
                int markedLine = TimingEditor::Get().GetMarkedLine();
                // Find first token of word
                int leadingSpaceInd = TimingEditor::Get().GetMarkedToken() - 1;
                while(leadingSpaceInd >= 0 && !doc.GetToken(markedLine, leadingSpaceInd).myValue.contains(' '))
                {
                    leadingSpaceInd--;
                }
                // Replace letters
                size_t spaceCharInd = doc.GetToken(markedLine, leadingSpaceInd).myValue.find_last_of(' ');
                if(leadingSpaceInd >= 0 && spaceCharInd < doc.GetLine(markedLine).size() - 1)
                {
                    doc.GetToken(markedLine, leadingSpaceInd).myValue[spaceCharInd + 1] = std::toupper(doc.GetToken(markedLine, leadingSpaceInd).myValue[spaceCharInd + 1]);
                }
                else
                {
                    doc.GetToken(markedLine, leadingSpaceInd + 1).myValue[0] = std::toupper(doc.GetToken(markedLine, leadingSpaceInd + 1).myValue[0]);
                }
            }
            if(ImGui::MenuItem("Toggle Letter Case"))
            {
                char markedChar = doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken()).myValue[TimingEditor::Get().GetMarkedChar()];
                doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken()).myValue[TimingEditor::Get().GetMarkedChar()] =
                    std::isupper(markedChar) ? std::tolower(markedChar) : std::toupper(markedChar);
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Effects", !g_isSafeMode))
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
            TimingEditor& timing = TimingEditor::Get();
            bool hasLineTag = false;
            bool hasNoEffectTag = false;
            if(doc.GetLine(timing.GetMarkedLine()).size() > 0)
            {
                hasLineTag = doc.GetToken(timing.GetMarkedLine(), 0).myValue.starts_with("<line");
            }
            if(doc.GetLine(timing.GetMarkedLine()).size() > (hasLineTag ? 1 : 0))
            {
                hasNoEffectTag = doc.GetToken(timing.GetMarkedLine(), (hasLineTag ? 1 : 0)).myValue.starts_with("<no effect>");
            }
            if(ImGui::BeginMenu("Image", !PreviewWindow::GetHasVideo() && PreviewWindow::GetBackgroundElementPaths().size() > 1))
            {
                int imgCount = PreviewWindow::GetBackgroundElementPaths().size();
                for(int i = 0; i < imgCount; i++)
                {
                    if(ImGui::MenuItem(PreviewWindow::GetBackgroundElementPaths()[i].data()))
                    {
                        uint imgTime = doc.GetThisOrNextTimedToken(timing.GetMarkedLine(), timing.GetMarkedToken()).myStartTime;
                        for(int line = timing.GetMarkedLine(); line < doc.GetData().size(); line++)
                        {
                            Serialization::KaraokeToken& compToken = doc.GetThisOrNextTimedToken(line, 0);
                            if(doc.IsNull(compToken)) {break;}
                            if(compToken.myStartTime >= imgTime)
                            {
                                Serialization::KaraokeLine& checkLine = doc.GetValidLineBefore(line);
                                if(!doc.IsNull(checkLine) && checkLine[0].myValue.starts_with("image "))
                                {
                                    line--;
                                }
                                Serialization::KaraokeToken newToken = {};
                                newToken.myValue = "";
                                newToken.myHasStart = true;
                                newToken.myStartTime = imgTime;
                                doc.GetData().insert(doc.GetData().begin() + line, {newToken});
                                newToken.myValue = "image 0.2 " + PreviewWindow::GetBackgroundElementPaths()[i];
                                newToken.myHasStart = false;
                                newToken.myStartTime = 0;
                                doc.GetData().insert(doc.GetData().begin() + line, {newToken});
                                doc.MakeDirty();
                                break;
                            }
                        }
                    }
                }
                ImGui::EndMenu();
            }
            if(ImGui::MenuItem("No Effect", "<no effect>", hasNoEffectTag))
            {
                if(hasNoEffectTag)
                {
                    doc.GetLine(timing.GetMarkedLine()).erase(doc.GetLine(timing.GetMarkedLine()).begin() + (hasLineTag ? 1 : 0));
                }
                else
                {
                    doc.GetLine(timing.GetMarkedLine()).insert(doc.GetLine(timing.GetMarkedLine()).begin() + (hasLineTag ? 1 : 0), {"<no effect>", false, 0});
                }
            }
            if(ImGui::MenuItem("Display Line", "<line#>", hasLineTag))
            {
                if(hasLineTag)
                {
                    doc.GetLine(timing.GetMarkedLine()).erase(doc.GetLine(timing.GetMarkedLine()).begin());
                }
                else
                {
                    doc.GetLine(timing.GetMarkedLine()).insert(doc.GetLine(timing.GetMarkedLine()).begin(), {"<line#1>", false, 0});
                }
            }
            ImGui::BeginDisabled();
            ImGui::SeparatorText("Text Effects");
            ImGui::EndDisabled();
            for(const auto& [alias, effect] : doc.GetEffectAliases())
            {
                if(ImGui::MenuItem(alias.data(), effect->myECHOValue.data()))
                {
                    Serialization::KaraokeToken& token = doc.GetToken(timing.GetMarkedLine(), timing.GetMarkedToken());
                    doc.GetLine(timing.GetMarkedLine()).insert(doc.GetLine(timing.GetMarkedLine()).begin() + timing.GetMarkedToken(), {("<" + alias + ">").data(), true, token.myStartTime});
                }
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Syllabify", !g_isSafeMode))
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
            if(ImGui::MenuItem("Touch Control", 0, WindowManager::GetWindow("Touch Control") != nullptr, !g_isSafeMode))
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
    if(g_shouldHideLoadingScreen)
    {
        g_shouldHideLoadingScreen = false;
        ImGui::Ext::StopLoadingScreen();
    }
    if(g_selfTestInProgress && !g_selfTestFailed)
    {
        FileHandler::SetLocalValue("Startup/FailCount", "0");
        g_selfTestInProgress = false;
    }
}

void DrawSelfTestWarningPopup()
{
    if(ImGui::BeginPopupModal("WARNING!##StartFail", 0, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped("Start up has failed multiple times in a row. To prevent another crash the project was not loaded. \n\n\n"
        "    You can choose to continue and try to load the files again. \n\n"
        "    If you have unsaved work you can enter Safe Mode and fix any errors. \n\n"
        "    If you cannot find anything wrong with the project and have saved any files you care about, you can choose Reset to remove all settings and the project.");
        ImGui::Dummy({5, DPI_SCALED(30)});
        if(ImGui::Button("Continue"))
        {
            Serialization::KaraokeDocument::Get().Load("/local", (
                Serialization::Preferences::HasKey("Document/FileID") ? Serialization::Preferences::GetString("Document/FileID") : ""
            ));
            ImGui::GetIO().IniFilename = "/local/Layout.Resonate";
            AudioPlayback::SetPlaybackFile("/local");
            PreviewWindow::AddBackgroundElement("/local/");
            g_selfTestFailed = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Safe Mode"))
        {
            WindowManager::DestroyWindow(WindowManager::GetWindow("Timing"));
            Serialization::KaraokeDocument::Get().Load("/local", (
                Serialization::Preferences::HasKey("Document/FileID") ? Serialization::Preferences::GetString("Document/FileID") : ""
            ));
            g_selfTestFailed = false;
            g_selfTestInProgress = false; // Open popup on next load as well
            g_isSafeMode = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Reset"))
        {
            for (auto &path : std::filesystem::directory_iterator("/local/"))
            {
                std::filesystem::remove(path);
            }
            FileHandler::SyncLocalFS();
            FileHandler::SetLocalValue("Startup/FailCount", "0");
            EM_ASM(location.reload(););
        }
        ImGui::EndPopup();
    }
    if(g_selfTestFailed)
    {
        ImGui::OpenPopup("WARNING!##StartFail");
    }
}

int main(){
    void* _window = nullptr;
    if(FileHandler::GetLocalValue("Startup/FailCount") != "")
    {
        int failCount = std::stoi(FileHandler::GetLocalValue("Startup/FailCount"));
        if(failCount > 4)
        {
            // Prevent data from loading because an error prevented the program from starting last time
            g_selfTestFailed = true;
        }
        FileHandler::SetLocalValue("Startup/FailCount", std::to_string(failCount + 1));
    }
    else
    {
        FileHandler::SetLocalValue("Startup/FailCount", "1");
    }
    MainWindow_Init("Resonate", &_window);
    MainWindow_StyleVarsShadow();
    MainWindow_StyleColorsShadow();
    ImGui::Ext::StartLoadingScreen(1, false);
    g_shouldHideLoadingScreen = true;
    Serialization::Syllabify_Init();
    if(!g_selfTestFailed)
    {
        Serialization::LoadPrefs();
    }
    //if(Serialization::Preferences::HasKey("Startup/FailCount"))
    //{
    //    int failCount = Serialization::Preferences::GetInt("Startup/FailCount");
    //    if(failCount > 4)
    //    {
    //        // Prevent data from loading because an error prevented the program from starting last time
    //        g_selfTestFailed = true;
    //    }
    //    Serialization::Preferences::SetInt("Startup/FailCount", failCount + 1);
    //}
    //else
    //{
    //    Serialization::Preferences::SetInt("Startup/FailCount", 1);
    //}
    if(!g_selfTestFailed)
    {
        Serialization::KaraokeDocument::Get().Load("/local", (
            Serialization::Preferences::HasKey("Document/FileID") ? Serialization::Preferences::GetString("Document/FileID") : ""
        ));
    }

    ImGui::Ext::SetShortcutEvents();
    
    ImGui::GetIO().IniFilename = "/local/Layout.Resonate";
    if(!std::filesystem::exists(ImGui::GetIO().IniFilename))
    {
        std::filesystem::copy_file("/imgui.ini", "/local/Layout.Resonate");
        FileHandler::SyncLocalFS();
    }
    if(g_selfTestFailed)
    {
        ImGui::GetIO().IniFilename = "/imgui.ini";
    }

    WindowManager::Init();
    WindowManager::AddWindow<TextEditor>("Raw Text");
    TimingEditor* timingEditor = WindowManager::AddWindow<TimingEditor>("Timing");
    WindowManager::AddWindow<AudioPlayback>("Audio");
    ImGui::SetWindowFocus("Timing");
    if(g_selfTestFailed)
    {
        AudioPlayback::SetPlaybackFile("-"); // Just to make sure the audio is not loaded.
    }

    if(!g_selfTestFailed)
    {
        PreviewWindow::AddBackgroundElement("/local/");
    }

    ImGui::GetIO().Fonts->AddFontDefault(nullptr);
    PreviewWindow::SetFont(ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/SCR1rahv RAGER HEVVY.otf", 40.0f));
    PreviewWindow::SetRulerFont(ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/SCR1rahv RAGER HEVVY.otf", 40.0f));
    ImFont* timingFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Fredoka-Regular.ttf", 40.0f);
    timingEditor->SetFont(timingFont);
    timingFont->Scale = .5f;
    MainWindow::Font = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Fredoka-Regular.ttf", 40.0f);
    MainWindow::Font->Scale = .5f;
    ImGui::GetIO().Fonts->Build();
    //ImGui::PushFont(roboto);

    emscripten_set_main_loop_arg(loop, (void*)_window, 0, false);
    return 0;
}