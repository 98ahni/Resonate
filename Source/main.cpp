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
#include "Extensions/Gamepad.h"
#include "Extensions/imguiExt.h"
#include "Extensions/FileHandler.h"
#include "Extensions/GoogleDrive.h"
#include "Extensions/Dropbox.h"
#include "Serialization/KaraokeData.h"
#include "Serialization/Syllabify.h"
#include "Serialization/Preferences.h"
#include "StringTools.h"
#include "Defines.h"
#include "EditMenu.h"
#include "GamepadActions.h"
#include <filesystem>

bool g_showInputDebugger = false;
char* g_testStr = new char[50];
extern "C" EMSCRIPTEN_KEEPALIVE void ShowInputDebugger() { g_showInputDebugger = true; }
EM_JS(void, show_input_debugger, (), {_ShowInputDebugger(); });

std::string g_userName = "(Anon)";
ImExtTexture g_userImage = {};
bool g_hasGoogleAcc = false;
bool g_hasDropboxAcc = false;
bool g_fileTabOpenedThisFrame = true; // Only use in File tab!
bool g_closeFileTab = false;
bool g_closeAboutTab = false;
bool g_shouldDeleteOnLoad = false;
bool g_hasCustomFont = false;
bool g_shouldRebuildFonts = false;
std::string g_customFontPath = "";
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
    if(folderPath == "")
    {
        g_closeFileTab = true;
        ImGui::Ext::StopLoadingScreen();
        return;
    }
    if(Serialization::KaraokeDocument::Get().Load(folderPath))
    {
        PreviewWindow::ClearBackgroundElements();
        if(g_hasCustomFont)
        {
            for (auto &path : std::filesystem::directory_iterator("/local/"))
            {
                if (path.path().extension() == ".ttf" || path.path().extension() == ".otf")
                {
                    std::error_code ferr;
                    std::filesystem::remove(path.path(), ferr);
                    g_hasCustomFont = false;
                    break;
                }
            }
        }
    }
    AudioPlayback::SetPlaybackFile(folderPath);
    PreviewWindow::AddBackgroundElement(folderPath);
    for (auto &path : std::filesystem::directory_iterator(folderPath))
    {
        if (path.path().extension() == ".ttf" || path.path().extension() == ".otf")
        {
            g_customFontPath = path.path().string();
            //ImGui::GetIO().Fonts->AddFontFromFileTTF(path.path().string().data(), 40.0f);
            //ImGui::GetIO().Fonts->AddFontFromFileTTF(path.path().string().data(), 40.0f);
            //if(!Serialization::Preferences::HasKey("Timing/CanUseCustomFont") || Serialization::Preferences::GetBool("Timing/CanUseCustomFont"))
            //{
            //    //ImGui::GetIO().Fonts->Fonts.erase(ImGui::GetIO().Fonts->Fonts.begin() + 2, ImGui::GetIO().Fonts->Fonts.end());
            //    ImFont* timingFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(path.path().string().data(), 40.0f);
            //    timingFont->Scale = .5f;
            //    //TimingEditor::Get().SetFont(timingFont);
            //}
            //else
            //{
            //    //ImGui::GetIO().Fonts->Fonts.erase(ImGui::GetIO().Fonts->Fonts.begin() + 2, ImGui::GetIO().Fonts->Fonts.end());
            //}
            //PreviewWindow::SetFont(ImGui::GetIO().Fonts->AddFontFromFileTTF(path.path().string().data(), 40.0f));
            //PreviewWindow::SetRulerFont(ImGui::GetIO().Fonts->AddFontFromFileTTF(path.path().string().data(), 40.0f));
            g_shouldRebuildFonts = true;
            g_hasCustomFont = true;
            std::filesystem::copy(path.path().string(), "/local", std::filesystem::copy_options::overwrite_existing);   
        }
    }
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
extern "C" EMSCRIPTEN_KEEPALIVE void ExportZip()
{
    ImGui::Ext::StartLoadingScreen();
    std::vector<std::string> pathList;
    pathList.push_back(Serialization::KaraokeDocument::Get().Save());
    pathList.push_back(AudioPlayback::GetPath());
    int imgCount = PreviewWindow::GetBackgroundElementPaths().size();
    for(int i = 0; i < imgCount; i++)
    {
        pathList.push_back("/local/" + PreviewWindow::GetBackgroundElementPaths()[i]);
    }
    if(g_hasCustomFont)
    {
        for (auto &path : std::filesystem::directory_iterator("/local/"))
        {
            if (path.path().extension() == ".ttf" || path.path().extension() == ".otf")
            {
                pathList.push_back(path.path().string());
                break;
            }
        }
    }
    FileHandler::DownloadZip(pathList, std::filesystem::path(Serialization::KaraokeDocument::Get().GetName()).replace_extension().c_str());
    Serialization::KaraokeDocument::Get().UnsetIsDirty();
    FileHandler::SyncLocalFS();
    g_closeFileTab = true;
    ImGui::Ext::StopLoadingScreen();
}

extern "C" EMSCRIPTEN_KEEPALIVE void GoogleTokenExpirationCallback(emscripten::EM_VAL aTime, emscripten::EM_VAL aUserName, emscripten::EM_VAL aProfilePhotoURL)
{
    Serialization::Preferences::SetDouble("Google/ExpirationDate", VAR_FROM_JS(aTime).as<double>());
    Serialization::Preferences::SetBool("Google/IsLoggedIn", true);
    g_userName = VAR_FROM_JS(aUserName).as<std::string>();
    std::string imgURL = VAR_FROM_JS(aProfilePhotoURL).as<std::string>();
    if(imgURL != "") ImGui::Ext::LoadImageFromURL("##ProfileImage", imgURL.data());
    else ImGui::Ext::LoadImageFromURL("##ProfileImage", "icons/ResonateIconLarge.png");
    ImGui::Ext::RenderTexture("##ProfileImage", g_userImage);
}
extern "C" EMSCRIPTEN_KEEPALIVE void LogInToGoogle()
{
    float expiration = Serialization::Preferences::HasKey("Google/ExpirationDate") ? Serialization::Preferences::GetDouble("Google/ExpirationDate") : 0;
    GoogleDrive::RequestToken(EM_ASM_DOUBLE({return Date.now();}) >= expiration, "_GoogleTokenExpirationCallback");
    g_closeFileTab = true;
}

extern "C" EMSCRIPTEN_KEEPALIVE void DropboxTokenExpirationCallback(emscripten::EM_VAL aTime, emscripten::EM_VAL aUserName, emscripten::EM_VAL aProfilePhotoURL)
{
    Serialization::Preferences::SetDouble("Dropbox/ExpirationDate", VAR_FROM_JS(aTime).as<double>());
    Serialization::Preferences::SetBool("Dropbox/IsLoggedIn", true);
    g_userName = VAR_FROM_JS(aUserName).as<std::string>();
    std::string imgURL = VAR_FROM_JS(aProfilePhotoURL).as<std::string>();
    if(imgURL != "") ImGui::Ext::LoadImageFromURL("##ProfileImage", imgURL.data());
    else ImGui::Ext::LoadImageFromURL("##ProfileImage", "icons/ResonateIconLarge.png");
    ImGui::Ext::RenderTexture("##ProfileImage", g_userImage);
}
extern "C" EMSCRIPTEN_KEEPALIVE void LogInToDropbox()
{
    float expiration = Serialization::Preferences::HasKey("Dropbox/ExpirationDate") ? Serialization::Preferences::GetDouble("Dropbox/ExpirationDate") : 0;
    Dropbox::RequestToken(true || EM_ASM_DOUBLE({return Date.now();}) >= expiration, "_DropboxTokenExpirationCallback"); // TODO: fix token refresh
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
        if(g_hasCustomFont)
        {
            for (auto &path : std::filesystem::directory_iterator("/local/"))
            {
                if (path.path().extension() == ".ttf" || path.path().extension() == ".otf")
                {
                    std::error_code ferr;
                    std::filesystem::remove(path.path(), ferr);
                    g_hasCustomFont = false;
                    break;
                }
            }
        }
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
    else if (path.extension() == ".ttf" || path.extension() == ".otf")
    {
        if(!Serialization::Preferences::HasKey("Timing/CanUseCustomFont") || Serialization::Preferences::GetBool("Timing/CanUseCustomFont"))
        {
            //ImGui::GetIO().Fonts->Fonts.erase(ImGui::GetIO().Fonts->Fonts.begin() + 2, ImGui::GetIO().Fonts->Fonts.end());
            ImFont* timingFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(path.string().data(), 40.0f);
            timingFont->Scale = .5f;
            TimingEditor::Get().SetFont(timingFont);
        }
        else
        {
            //ImGui::GetIO().Fonts->Fonts.erase(ImGui::GetIO().Fonts->Fonts.begin() + 2, ImGui::GetIO().Fonts->Fonts.end());
        }
        PreviewWindow::SetFont(ImGui::GetIO().Fonts->AddFontFromFileTTF(path.string().data(), 40.0f));
        PreviewWindow::SetRulerFont(ImGui::GetIO().Fonts->AddFontFromFileTTF(path.string().data(), 40.0f));
        g_shouldRebuildFonts = true;
        g_hasCustomFont = true;
        std::filesystem::copy(path.string(), "/local", std::filesystem::copy_options::overwrite_existing);
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
    Gamepad::Update();
    if(g_shouldRebuildFonts)
    {
        ImGui::GetIO().Fonts->Clear();
        ImGui::GetIO().Fonts->AddFontDefault(nullptr);
        MainWindow::Font = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Fredoka-Regular.ttf", 40.0f);
        MainWindow::Font->Scale = .5f;
        PreviewWindow::SetFont(ImGui::GetIO().Fonts->AddFontFromFileTTF(g_customFontPath.data(), 40.0f));
        PreviewWindow::SetRulerFont(ImGui::GetIO().Fonts->AddFontFromFileTTF(g_customFontPath.data(), 40.0f));
        ImFont* timingCustomFont = nullptr;
        if(!Serialization::Preferences::HasKey("Timing/CanUseCustomFont") || Serialization::Preferences::GetBool("Timing/CanUseCustomFont"))
        {
            timingCustomFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(g_customFontPath.data(), 40.0f);
            timingCustomFont->Scale = .5f;
        }
        ImFont* timingFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Fredoka-Regular.ttf", 40.0f);
        timingFont->Scale = .5f;
        TimingEditor::Get().SetFont(timingFont, timingCustomFont);
        MainWindow_Invalidate();
        ImGui::GetIO().Fonts->Build();
        g_shouldRebuildFonts = false;
    }
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
                g_hasGoogleAcc = GoogleDrive::HasToken() && EM_ASM_DOUBLE({return Date.now();}) <= Serialization::Preferences::GetDouble("Google/ExpirationDate");
                g_hasDropboxAcc = Dropbox::HasToken() && 
                    //(!Serialization::Preferences::HasKey("Dropbox/ExpirationDate") ||
                    EM_ASM_DOUBLE({return Date.now();}) <= Serialization::Preferences::GetDouble("Dropbox/ExpirationDate");
            }
            if(!g_hasGoogleAcc && !g_hasDropboxAcc)
            {
                ImGui::MenuItem("Log In With Google", 0, false, GoogleDrive::Ready());
                if(GoogleDrive::Ready()) { ImGui::Ext::CreateHTMLButton("GoogleLogin", "click", "_LogInToGoogle"); }
                ImGui::MenuItem("Log In to Dropbox");
                ImGui::Ext::CreateHTMLButton("DropboxLogin", "click", "_LogInToDropbox");
                ImGui::Separator();
                ImGui::MenuItem("Open Project", 0, false, !g_isSafeMode);
                if(!g_isSafeMode){ImGui::Ext::CreateHTMLButton("OpenProject", "click", "_LoadProject");}
                ImGui::MenuItem("Save Document");
                ImGui::Ext::CreateHTMLButton("SaveProject", "click", "_SaveProject");
                ImGui::Separator();
            }
            else if(g_hasGoogleAcc)
            {
                ImGui::SeparatorText("Google Drive");
                // Profile info + logout
                if(g_userImage.myID != 0)
                {
                    ImGui::Image(g_userImage.myID, {ImGui::GetTextLineHeightWithSpacing(), ImGui::GetTextLineHeightWithSpacing()});
                    ImGui::SameLine();
                }
                ImGui::Text(g_userName.data());
                if(ImGui::MenuItem("Log Out"))
                {
                    GoogleDrive::LogOut();
                }
                ImGui::Separator();
                if(ImGui::MenuItem("Open Project", 0, false, !g_isSafeMode))
                {
                    g_shouldDeleteOnLoad = true;
                    ImGui::Ext::StartLoadingScreen();
                    GoogleDrive::LoadProject("application/vnd.google-apps.folder", "_LoadFileFromCloudDrive", "_LoadCompletedFromCloudDrive", "_LoadCanceledFromCloudDrive");
                }
                if(ImGui::MenuItem("Save Document", 0, false, doc.GetFileID() != ""))
                {
                    ImGui::Ext::StartLoadingScreen();
                    GoogleDrive::SaveProject(doc.GetFileID(), doc.Save());
                    Serialization::KaraokeDocument::Get().UnsetIsDirty();
                    FileHandler::SyncLocalFS();
                    ImGui::Ext::StopLoadingScreen();
                }
                ImGui::SeparatorText("Local");
            }
            else if(g_hasDropboxAcc)
            {
                ImGui::SeparatorText("Dropbox");
                // Profile info + logout
                if(g_userImage.myID != 0)
                {
                    ImGui::Image(g_userImage.myID, {ImGui::GetTextLineHeightWithSpacing(), ImGui::GetTextLineHeightWithSpacing()});
                    ImGui::SameLine();
                }
                ImGui::Text(g_userName.data());
                if(ImGui::MenuItem("Log Out"))
                {
                    Dropbox::LogOut();
                }
                ImGui::Separator();
                ImGui::MenuItem("Open Project", 0, false, !g_isSafeMode);
                if(!g_isSafeMode) {ImGui::Ext::CreateHTMLButton("OpenDBProject", "click", "_OpenDropboxChooser");}
                if(ImGui::MenuItem("Save Document", 0, false, doc.GetFileID() != ""))
                {
                    ImGui::Ext::StartLoadingScreen();
                    Dropbox::SaveProject(doc.GetFileID(), doc.Save());
                    Serialization::KaraokeDocument::Get().UnsetIsDirty();
                    FileHandler::SyncLocalFS();
                    ImGui::Ext::StopLoadingScreen();
                }
                ImGui::SeparatorText("Local");
            }
            ImGui::MenuItem("Export as Zip");
            ImGui::Ext::CreateHTMLButton("ExportZip", "click", "_ExportZip");
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
            ImGui::Ext::DestroyHTMLElement("ExportZip");
        }
        Menu::Edit_CheckShortcuts();
        if(ImGui::BeginMenu("Edit", !g_isSafeMode))
        {
            if(ImGui::MenuItem("Insert Line Break", "Alt + Enter", false, !TimingEditor::Get().GetInputUnsafe()))
            {
                Menu::Edit_InsertLinebreak();
            }
            if(ImGui::MenuItem("Merge Line Up", "Alt + Left", false, !TimingEditor::Get().GetInputUnsafe()))
            {
                Menu::Edit_MergeLineUp();
            }
            if(ImGui::MenuItem("Merge Line Down", "Alt + Right", false, !TimingEditor::Get().GetInputUnsafe()))
            {
                Menu::Edit_MergeLineDown();
            }
            if(ImGui::MenuItem("Move Line Up", "Alt + Up", false, !TimingEditor::Get().GetInputUnsafe()))
            {
                Menu::Edit_MoveLineUp();
            }
            if(ImGui::MenuItem("Move Line Down", "Alt + Down", false, !TimingEditor::Get().GetInputUnsafe()))
            {
                Menu::Edit_MoveLineDown();
            }
            if(ImGui::MenuItem("Duplicate Line", "Alt + Space", false, !TimingEditor::Get().GetInputUnsafe()))
            {
                Menu::Edit_DuplicateLine();
            }
            ImGui::Separator();
            if(ImGui::MenuItem("Remove Line", "Alt + Backspace", false, !TimingEditor::Get().GetInputUnsafe()))
            {
                Menu::Edit_RemoveLine();
            }
            ImGui::Separator();
            ImGui::SeparatorText("Word Case");
            if(ImGui::MenuItem("Majuscule", "(EX,AM,PLE) Alt + Shift + Up", false, !TimingEditor::Get().GetInputUnsafe()))
            {
                Menu::Edit_Majuscule();
            }
            if(ImGui::MenuItem("Minuscule", "(ex,am,ple) Alt + Shift + Down", false, !TimingEditor::Get().GetInputUnsafe()))
            {
                Menu::Edit_Minuscule();
            }
            if(ImGui::MenuItem("Capital", "(Ex,am,ple) Alt + Shift + Left", false, !TimingEditor::Get().GetInputUnsafe()))
            {
                Menu::Edit_Capital();
            }
            if(ImGui::MenuItem("Toggle Letter Case", "Alt + Shift + Right", false, !TimingEditor::Get().GetInputUnsafe()))
            {
                Menu::Edit_ToggleCase();
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
            ImGui::SeparatorText("Line Effects");
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
                    if(ImGui::MenuItem(PreviewWindow::GetBackgroundElementPaths()[i].data(), 0, false, !TimingEditor::Get().GetInputUnsafe()))
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
            if(ImGui::MenuItem("No Effect", "<no effect>", hasNoEffectTag, !TimingEditor::Get().GetInputUnsafe()))
            {
                if(hasNoEffectTag)
                {
                    doc.GetLine(timing.GetMarkedLine()).erase(doc.GetLine(timing.GetMarkedLine()).begin() + (hasLineTag ? 1 : 0));
                }
                else
                {
                    doc.GetLine(timing.GetMarkedLine()).insert(doc.GetLine(timing.GetMarkedLine()).begin() + (hasLineTag ? 1 : 0), {"<no effect>", false, 0});
                }
                doc.MakeDirty();
            }
            if(ImGui::MenuItem("Display Line", "<line#>", hasLineTag, !TimingEditor::Get().GetInputUnsafe()))
            {
                if(hasLineTag)
                {
                    doc.GetLine(timing.GetMarkedLine()).erase(doc.GetLine(timing.GetMarkedLine()).begin());
                }
                else
                {
                    doc.GetLine(timing.GetMarkedLine()).insert(doc.GetLine(timing.GetMarkedLine()).begin(), {"<line#1>", false, 0});
                }
                doc.MakeDirty();
            }
            ImGui::BeginDisabled();
            ImGui::SeparatorText("Text Effects");
            ImGui::EndDisabled();
            for(const auto& [alias, effect] : doc.GetEffectAliases())
            {
                if(ImGui::MenuItem(alias.data(), effect->myECHOValue.data(), false, !TimingEditor::Get().GetInputUnsafe()))
                {
                    Serialization::KaraokeToken& token = doc.GetToken(timing.GetMarkedLine(), timing.GetMarkedToken());
                    doc.GetLine(timing.GetMarkedLine()).insert(doc.GetLine(timing.GetMarkedLine()).begin() + timing.GetMarkedToken(), {("<" + alias + ">").data(), true, token.myStartTime});
                    doc.MakeDirty();
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
                        doc.Parse("[00:00:00]" + text);
                        if(doc.GetData().front().size() == 1 && doc.IsPauseToken(doc.GetData().front().front()))
                        {
                            doc.GetData().front().erase(doc.GetData().front().begin());
                        }
                        doc.MakeDirty();
                    }
                }
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("Line", !TimingEditor::Get().GetInputUnsafe()))
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
            if(ImGui::MenuItem("Raw Text", 0, WindowManager::GetWindow("Raw Text") != nullptr))
            {
                if(WindowManager::GetWindow("Raw Text") != nullptr)
                {
                    WindowManager::DestroyWindow(WindowManager::GetWindow("Raw Text"));
                }
                else
                {
                    WindowManager::AddWindow<TextEditor>("Raw Text");
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
            if(ImGui::MenuItem("Show Input Debugger"))
            {
                ShowInputDebugger();
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
        for(int i = 0; i < 25; i++)
        {
            if(Gamepad::GetButton((Gamepad::Button)i))
            {
                ImGui::Text("[std%i,%f]", i, Gamepad::GetButtonAnalogRaw((Gamepad::Button)i));
                ImGui::SameLine();
            }
        }
        ImGui::NewLine();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 startPos = ImGui::GetCursorPos();
        startPos.x += ImGui::GetWindowPos().x;
        startPos.y += ImGui::GetWindowPos().y;
        drawList->PathLineTo({startPos.x + 30, startPos.y});
        drawList->PathLineTo({startPos.x + 30, startPos.y + 60});
        drawList->PathStroke(IM_COL32(255, 0, 50, 255), 0, 1);
        drawList->PathLineTo({startPos.x + 100, startPos.y});
        drawList->PathLineTo({startPos.x + 100, startPos.y + 60});
        drawList->PathStroke(IM_COL32(255, 0, 50, 255), 0, 1);
        drawList->PathLineTo({startPos.x, startPos.y + 30});
        drawList->PathLineTo({startPos.x + 60, startPos.y + 30});
        drawList->PathStroke(IM_COL32(255, 0, 50, 255), 0, 1);
        drawList->PathLineTo({startPos.x + 70, startPos.y + 30});
        drawList->PathLineTo({startPos.x + 130, startPos.y + 30});
        drawList->PathStroke(IM_COL32(255, 0, 50, 255), 0, 1);
        drawList->AddCircleFilled({startPos.x + 30 + (Gamepad::GetAxis(Gamepad::LeftStickX) * 30), startPos.y + 30 + (Gamepad::GetAxis(Gamepad::LeftStickY) * -30)}, 5, IM_COL32(155, 0, 200, 255));
        drawList->AddCircleFilled({startPos.x + 30 + (Gamepad::GetAxisRaw(Gamepad::LeftStickX) * 30), startPos.y + 30 + (Gamepad::GetAxisRaw(Gamepad::LeftStickY) * -30)}, 5, IM_COL32_WHITE);
        drawList->AddCircleFilled({startPos.x + 100 + (Gamepad::GetAxis(Gamepad::RightStickX) * 30), startPos.y + 30 + (Gamepad::GetAxis(Gamepad::RightStickY) * -30)}, 5, IM_COL32(155, 0, 200, 255));
        drawList->AddCircleFilled({startPos.x + 100 + (Gamepad::GetAxisRaw(Gamepad::RightStickX) * 30), startPos.y + 30 + (Gamepad::GetAxisRaw(Gamepad::RightStickY) * -30)}, 5, IM_COL32_WHITE);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 60);
        ImGui::Text("[ x: %f,  y: %f]   [ x: %f,  y: %f]", Gamepad::GetAxis(Gamepad::LeftStickX), Gamepad::GetAxis(Gamepad::LeftStickY), Gamepad::GetAxis(Gamepad::RightStickX), Gamepad::GetAxis(Gamepad::RightStickY));
        ImGui::Text("[rx: %f, ry: %f]   [rx: %f, ry: %f]", Gamepad::GetAxisRaw(Gamepad::LeftStickX), Gamepad::GetAxisRaw(Gamepad::LeftStickY), Gamepad::GetAxisRaw(Gamepad::RightStickX), Gamepad::GetAxisRaw(Gamepad::RightStickY));
        ImGui::Text("[ m: %f, sd: %f]   [ m: %f, sd: %f]", Gamepad_Magnitude(Gamepad::LeftStick), Gamepad_Spin(Gamepad::LeftStick), Gamepad_Magnitude(Gamepad::RightStick), Gamepad_Spin(Gamepad::RightStick));
        //char* logs = &get_console_logs();
        //ImGui::Text(logs);
        //free(logs);

        ImGui::Image(MainWindow::Font->ContainerAtlas->TexID, {ImGui::GetWindowHeight() - ImGui::GetCursorPosY(), ImGui::GetWindowHeight() - ImGui::GetCursorPosY()});
        ImGui::End();
    }

    WindowManager::ImGuiDraw();
    DoGamepadActions();

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
    Gamepad::Initialize();
    Gamepad::SetDeadZone(.9f);
    
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
    //WindowManager::AddWindow<TextEditor>("Raw Text");
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

    g_customFontPath = "Fonts/SCR1rahv RAGER HEVVY.otf";
    for (auto &path : std::filesystem::directory_iterator("/local/"))
    {
        if (path.path().extension() == ".ttf" || path.path().extension() == ".otf")
        {
            g_hasCustomFont = true;
            g_customFontPath = path.path().string();
            break;
        }
    }

    ImGui::GetIO().Fonts->AddFontDefault(nullptr);
    MainWindow::Font = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Fredoka-Regular.ttf", 40.0f);
    MainWindow::Font->Scale = .5f;
    PreviewWindow::SetFont(ImGui::GetIO().Fonts->AddFontFromFileTTF(g_customFontPath.data(), 40.0f));
    PreviewWindow::SetRulerFont(ImGui::GetIO().Fonts->AddFontFromFileTTF(g_customFontPath.data(), 40.0f));
    ImFont* timingFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Fredoka-Regular.ttf", 40.0f);
    ImFont* timingCustomFont = nullptr;
    if(!Serialization::Preferences::HasKey("Timing/CanUseCustomFont") || Serialization::Preferences::GetBool("Timing/CanUseCustomFont"))
    {
        timingCustomFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(g_customFontPath.data(), 40.0f);
        timingCustomFont->Scale = .5f;
    }
    timingFont->Scale = .5f;
    timingEditor->SetFont(timingFont, timingCustomFont);
    ImGui::GetIO().Fonts->Build();

    emscripten_set_main_loop_arg(loop, (void*)_window, 0, false);
    return 0;
}