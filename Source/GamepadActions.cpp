//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024-2025 98ahni> Original file author

#include "GamepadActions.h"
#include "Extensions/imguiExt.h"
#include <misc/cpp/imgui_stdlib.h>
#include "Serialization/KaraokeData.h"
#include <Serialization/Preferences.h>
#include "Windows/MainWindow.h"
#include "Windows/TimingEditor.h"
#include "Windows/Settings.h"
#include "Windows/AudioPlayback.h"
#include "Windows/Preview.h"
#include "Windows/Properties.h"
#include "Defines.h"
#include "StringTools.h"
#include "EditMenu.h"
using namespace HUDSprite;

enum Layer
{
    Standard, Settings, Effects, Layout, Adjust, Recover
};
Layer g_layerLastFrame;
float g_currentSpin = 0;
bool g_validSpin = false;

enum EffectSpinType : char {Singer, Image};
enum MenuSpinType : char {Document, Local};
EffectSpinType g_effectSpinType = Singer;
MenuSpinType g_menuSpinType = Document;

bool g_hasLocalEffects = false;
bool g_hasInitMetronome = false;
std::string g_editingEffectName = "";
std::vector<std::string> g_localEffectNames = {};
std::vector<std::string> g_docEffectNames = {};
PropertiesWindow* g_properties = nullptr;
int g_lastAddedEffect = -1;
int g_selectedColorSlider = 0;
int g_imageShiftTime = 0;
int g_imageFadeTime = 0;

ImExtTexture g_menuBGTexture = {};
ImExtTexture g_hudTexture = {};
bool g_showOverlay = true;
ImVec2 g_hudStartUVs[] = {
    {.0f, .0f}, {.0f, .1f}, {.0f, .2f}, {.0f, .3f}, {.0f, .4f},
    {.1f, .0f}, {.2f, .0f}, {.3f, .0f}, {.4f, .0f}, {.5f, .0f},
    {.6f, .0f}, {.6f, .1f}, {.5f, .1f}, {.4f, .1f}, {.3f, .1f}, {.2f, .1f}, {.1f, .1f},
    {.1f, .2f}, {.2f, .2f}, {.3f, .2f}, {.4f, .2f}, {.5f, .2f}, {.6f, .2f}, {.6f, .3f},
    {.1f, .3f}, {.2f, .3f}, {.3f, .3f}, {.4f, .3f}, {.5f, .3f},
    {.1f, .4f}, {.2f, .4f}, {.3f, .4f}, {.4f, .4f}, {.5f, .4f}, {.6f, .4f}, {.5f, .5f}, {.6f, .5f},
    {.7f, .0f}, {.4f, .6f}, {.7f, .3f}, {.7f, .6f}, {.0f, .5f}, {.0f, .7f},
    {.2f, .7f}, {.2f, .5f}, {.3f, .7f}, {.2f, .6f},
    //{.3f, .85f}, {.35f, .6f}, {.4f, .85f}, {.35f, .7f},
    {.4f, .9f}, {.5f, .9f}, {.6f, .9f}, {.7f, .9f},
    {.0f, .9f}, {.2f, .9f}, {.3f, .85f}, {.9f, .9f},
};
ImVec2 g_hudEndUVs[] = {
    {.1f, .1f}, {.1f, .2f}, {.1f, .3f}, {.1f, .4f}, {.1f, .5f},
    {.2f, .1f}, {.3f, .1f}, {.4f, .1f}, {.5f, .1f}, {.6f, .1f},
    {.7f, .1f}, {.7f, .2f}, {.6f, .2f}, {.5f, .2f}, {.4f, .2f}, {.3f, .2f}, {.2f, .2f},
    {.2f, .3f}, {.3f, .3f}, {.4f, .3f}, {.5f, .3f}, {.6f, .3f}, {.7f, .3f}, {.7f, .4f},
    {.2f, .4f}, {.3f, .4f}, {.4f, .4f}, {.5f, .4f}, {.6f, .4f},
    {.2f, .5f}, {.3f, .5f}, {.4f, .5f}, {.5f, .5f}, {.6f, .5f}, {.7f, .5f}, {.6f, .6f}, {.7f, .6f},
    {1.f, .3f}, {.7f, .9f}, {1.f, .6f}, {1.f, .9f}, {.2f, .7f}, {.2f, .9f},
    {.3f, .85f}, {.35f, .6f}, {.4f, .85f}, {.35f, .7f},
    //{.2f, .7f}, {.2f, .5f}, {.3f, .7f}, {.2f, .6f},
    {.5f, 1.f}, {.6f, 1.f}, {.7f, 1.f}, {.8f, 1.f},
    {.2f, 1.f}, {.3f, 1.f}, {.4f, 1.f}, {1.f, 1.f},
};

void SetUpLocalEffectData()
{
    if(!g_properties) { g_properties = new PropertiesWindow(); }
    g_localEffectNames.clear();
    for(const auto& [alias, effect] : g_properties->GetLocalEffectAliases())
    {
        g_localEffectNames.push_back(alias);
    }
}
void SetUpDocumentEffectData()
{
    g_docEffectNames.clear();
    for(const auto& [alias, effect] : Serialization::KaraokeDocument::Get().GetEffectAliases())
    {
        g_docEffectNames.push_back(alias);
    }
}

void CheckGamepadActions();
void DoStandardAdjustActions();
void DoSettingsActions();
void DoEffectsActions();
void DoLayoutActions();
int StickMenu(float aScroll, std::vector<std::string> someLabels);
bool DrawImagePopup();
bool DrawLatencyPopup();
bool DrawFontSizePopup();
bool DrawDocColorsPopup();
bool DrawTimeShiftPopup();
bool DrawSingerPopup();
void DrawOverlay();

void DrawHudSprite(HUDSprite::HUDSpriteID aSprite, ImVec2 aSize, ImU32 aColor)
{
    ImGui::Image(g_hudTexture.myID, aSize, g_hudStartUVs[aSprite], g_hudEndUVs[aSprite], ImGui::ColorConvertU32ToFloat4(aColor));
}

void AddHudSpriteTo(ImDrawList *aDrawList, HUDSprite::HUDSpriteID aSprite, ImVec2 aScreenPosition, ImVec2 aSize, ImU32 aColor)
{
    aDrawList->AddImage(g_hudTexture.myID, aScreenPosition, {aScreenPosition.x + aSize.x, aScreenPosition.y + aSize.y}, g_hudStartUVs[aSprite], g_hudEndUVs[aSprite], aColor);
}

void DoGamepadActions()
{
    if(Gamepad::GetCount() == 0) { return; }
    if(g_hudTexture.myID == 0)
    {
        ImGui::Ext::LoadImage("##MenuHUDTexture", "GamepadSprites/IconSprites.png");
        ImGui::Ext::RenderTexture("##MenuHUDTexture", g_hudTexture);
    }
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetColorU32(ImGuiCol_WindowBg));
    if( DrawImagePopup() ||
        DrawLatencyPopup() ||
        DrawFontSizePopup() ||
        DrawDocColorsPopup() ||
        DrawTimeShiftPopup() ||
        DrawSingerPopup())
    {
        ImGui::PopStyleColor();
        TimingEditor::Get().SetInputUnsafe(true);
        return;
    }
    TimingEditor::Get().SetInputUnsafe(false);
    ImGui::PopStyleColor();
    DoStandardAdjustActions();
    DoSettingsActions();
    DoEffectsActions();
    DoLayoutActions();
    CheckGamepadActions();
    if(!g_showOverlay) { return; }
    DrawOverlay();
}

void CheckGamepadActions()
{
    /*
    R Stick spin - scroll

    Modifiers (affects D-pad, L-stick, [Confirm], [Decline], L, R)
    - ZL
    - ZR
    - [Menu]
    - [Action]
    
    Focus windows
    - Image
    - Latency
    - Change Font Size
    - Doc Colors
    - Shift Timings
    - Singer

    Standard
    - D-pad = arrows
    - [Confirm] - Set start time
    - [Decline] - Set end time
    - L/R - Change speed
    - Audio (L stick)
    - - Click - Stop
    - - Flick Down - Play/Pause
    - - Flick Up - Progress to marker
    - - Flick Left/Right - RW/FF 5s
    
    Quick settings ([Menu])
    - Tap - Hide/Show overlay
    - Hold
    - - [Confirm] - Open/Close Preview
    - - D-pad Up - Open Latency
    - - D-pad Left - Open Change Font Size
    - - D-pad Right - Open Doc Colors
    - - D-pad Down - Open Shift Timings
    - - L-stick spin - Cycle doc/local singers (release to open Singer window)
    - - L-stick click - Toggle doc/local singers menu

    Effects ([Action])
    - Tap
    - - if on image - Open Image
    - - if on effect - Remove effect
    - - else - Add last chosen effect
    - Hold 
    - - D-pad Up/Down - Set/Adjust line tag for marked line
    - - D-pad Left - 
    - - D-pad Right - 
    - - [Decline] - Toggle <no effect>
    - - L-stick spin - Cycle singers/images (release to use)
    - - L-stick click - Toggle singers/images menu
    
    Layout (ZL)
    - D-pad Up/Down - Move line
    - D-pad Left/Right - Merge line up/down
    - [Confirm] - Duplicate line
    - [Decline] - Insert linebreak
    - L-stick click - RemoveLine
    - L-stick Flick Up - Majuscule case
    - L-stick Flick Down - Minuscule case
    - L-stick Flick Left - Capital case
    - L-stick Flick Right - Toggle letter case
    
    Adjust (ZR)
    - D-pad = arrows
    - [Confirm] - Split/Join
    */

    if(!Gamepad::GetButton(Gamepad::Square) && !Gamepad::GetButton(Gamepad::Triangle) && !Gamepad::GetButton(Gamepad::L2) && !Gamepad::GetButton(Gamepad::R2))
    {
        g_layerLastFrame = Standard;
    }
    if(Gamepad::GetButton(Gamepad::Square) && !Gamepad::GetButton(Gamepad::Triangle) && !Gamepad::GetButton(Gamepad::L2) && !Gamepad::GetButton(Gamepad::R2))
    {
        g_layerLastFrame = Settings;
    }
    if(!Gamepad::GetButton(Gamepad::Square) && Gamepad::GetButton(Gamepad::Triangle) && !Gamepad::GetButton(Gamepad::L2) && !Gamepad::GetButton(Gamepad::R2))
    {
        g_layerLastFrame = Effects;
    }
    if(!Gamepad::GetButton(Gamepad::Square) && !Gamepad::GetButton(Gamepad::Triangle) && Gamepad::GetButton(Gamepad::L2) && !Gamepad::GetButton(Gamepad::R2))
    {
        g_layerLastFrame = Layout;
    }
    if(!Gamepad::GetButton(Gamepad::Square) && !Gamepad::GetButton(Gamepad::Triangle) && !Gamepad::GetButton(Gamepad::L2) && Gamepad::GetButton(Gamepad::R2))
    {
        g_layerLastFrame = Adjust;
    }
}

void DoStandardAdjustActions()
{
    if(g_layerLastFrame == Standard || g_layerLastFrame == Adjust)
    {
        Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
        if(Gamepad::GetButtonDown(Gamepad::D_Up)) TimingEditor::Get().MoveMarkerUp();
        if(Gamepad::GetButtonDown(Gamepad::D_Down)) TimingEditor::Get().MoveMarkerDown();
        if(Gamepad::GetButtonDown(Gamepad::D_Left)) TimingEditor::Get().MoveMarkerLeft(g_layerLastFrame == Adjust);
        if(Gamepad::GetButtonDown(Gamepad::D_Right)) TimingEditor::Get().MoveMarkerRight(g_layerLastFrame == Adjust);
        if(Gamepad::GetButtonDown(Gamepad::Circle)) TimingEditor::Get().RecordEndTime();
        if(Gamepad::GetButtonDown(Gamepad::Cross))
        {
            if(g_layerLastFrame == Standard) TimingEditor::Get().RecordStartTime();
            else TimingEditor::Get().ToggleTokenHasTime();
            History::ForceEndRecord();
        }

        if(Gamepad::GetButtonDown(Gamepad::L1)) AudioPlayback::SetPlaybackSpeed(AudioPlayback::GetPlaybackSpeed() - 1);
        if(Gamepad::GetButtonDown(Gamepad::R1)) AudioPlayback::SetPlaybackSpeed(AudioPlayback::GetPlaybackSpeed() + 1);
        if(Gamepad::GetButtonDown(Gamepad::LeftStick)) AudioPlayback::Stop();
        if(Gamepad_FlickAxis(Gamepad::LeftStickX, Gamepad_FlickRIGHT))
        {
            AudioPlayback::SetPlaybackProgress(AudioPlayback::GetPlaybackProgress() + (500.f * (AudioPlayback::GetPlaybackSpeed() * .1f)));
        }
        if(Gamepad_FlickAxis(Gamepad::LeftStickX, Gamepad_FlickLEFT))
        {
            AudioPlayback::SetPlaybackProgress(AudioPlayback::GetPlaybackProgress() - (500.f * (AudioPlayback::GetPlaybackSpeed() * .1f)));
        }
        if(Gamepad_FlickAxis(Gamepad::LeftStickY, Gamepad_FlickDOWN))
        {
            if(AudioPlayback::GetIsPlaying())
            {
                AudioPlayback::Pause();
            }
            else
            {
                AudioPlayback::Play();
            }
        }
        if(Gamepad_FlickAxis(Gamepad::LeftStickY, Gamepad_FlickUP))
        {
            AudioPlayback::SetPlaybackProgress(doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken()).myStartTime);
        }
    }
}

void TogglePreview()        // These two functions has to be separated out to make DoSettingsActions() fit in a 4kB stack.
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
std::vector<std::string> ConstructOptions()
{
    std::vector<std::string> options = g_menuSpinType == Document ? g_docEffectNames : g_localEffectNames;
    options.emplace(options.begin(), "Add New");
    options.emplace(options.begin(), "Cancel");
    return options;
}
void DoSettingsActions()
{
    if(g_layerLastFrame == Settings)
    {
        Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
        if(Gamepad_Tap(Gamepad::Square))
        {
            // Show/Hide overlay
            g_showOverlay = !g_showOverlay;
        }
        else if(Gamepad_Hold(Gamepad::Square))
        {
            if(Gamepad::GetButtonDown(Gamepad::D_Up)) { g_hasInitMetronome = false; ImGui::OpenPopup("Latency##Gamepad"); }
            if(Gamepad::GetButtonDown(Gamepad::D_Down)) { ImGui::OpenPopup("Shift Timings##Gamepad"); }
            if(Gamepad::GetButtonDown(Gamepad::D_Left)) { ImGui::OpenPopup("Font Size##Gamepad"); }
            if(Gamepad::GetButtonDown(Gamepad::D_Right)) { ImGui::OpenPopup("Default Colors##Gamepad"); }
            if(Gamepad::GetButtonDown(Gamepad::Cross))
            {
                TogglePreview();
            }
            if(Gamepad::GetButtonDown(Gamepad::LeftStick)) {g_menuSpinType = (MenuSpinType)!g_menuSpinType;}
            float mag = Gamepad_Magnitude(Gamepad::LeftStick);
            std::vector<std::string> options = ConstructOptions();
            //std::vector<std::string> options = g_menuSpinType == Document ? g_docEffectNames : g_localEffectNames;
            //options.emplace(options.begin(), "Add New");
            //options.emplace(options.begin(), "Cancel");
            float adjustedSpin = g_currentSpin + .5f;
            while(adjustedSpin < 0) { adjustedSpin += options.size();}
            int index = ((int)adjustedSpin) % options.size();
            if(mag > .8f)
            {
                if(!g_validSpin) { SetUpDocumentEffectData(); SetUpLocalEffectData(); }
                ImDrawList* drawList = ImGui::GetForegroundDrawList();
                ImVec2 origin = {((float)MainWindow::SwapWidth) * .5f, ((float)MainWindow::SwapHeight) * .5f};
                float halfSquare = origin.x < origin.y ? origin.x : origin.y;
                g_validSpin = true;
                ImGui::PushFont(MainWindow::Font);
                if(index == 0)
                {
                    // Cancel symbol
                }
                else if(index == 1)
                {
                    // Add symbol
                }
                else
                {
                    Serialization::KaraokeColorEffect* effect = (Serialization::KaraokeColorEffect*)((g_menuSpinType == Document ? doc.GetEffectAliases() : g_properties->GetLocalEffectAliases()).at(options[index]));
                    if(effect->myHasEndColor)
                    {
                        drawList->AddRectFilled(
                            {origin.x - (halfSquare * .3f), origin.y - (halfSquare * .3f)},
                            {origin.x - (halfSquare * .1f), origin.y - (halfSquare * .1f)},
                            IM_COL32_FROM_DOC(effect->myStartColor), DPI_SCALED(15));
                        drawList->AddRectFilled(
                            {origin.x + (halfSquare * .1f), origin.y - (halfSquare * .3f)},
                            {origin.x + (halfSquare * .3f), origin.y - (halfSquare * .1f)},
                            IM_COL32_FROM_DOC(effect->myEndColor), DPI_SCALED(15));
                    }
                    else
                    {
                        drawList->AddRectFilled(
                            {origin.x - (halfSquare * .1f), origin.y - (halfSquare * .3f)},
                            {origin.x + (halfSquare * .1f), origin.y - (halfSquare * .1f)},
                            IM_COL32_FROM_DOC(effect->myStartColor), DPI_SCALED(15));
                    }
                    ImVec2 textSize = ImGui::CalcTextSize(effect->myECHOValue.data());
                    drawList->AddText(MainWindow::Font, DPI_SCALED(30), {origin.x - (textSize.x * .75f), (origin.y * 1.1f) - (textSize.y * .75f)}, ImGui::GetColorU32(ImGuiCol_TextDisabled), effect->myECHOValue.data());
                }
                ImVec2 textSize = ImGui::CalcTextSize(g_menuSpinType == Document ? "Document Effects" : "Local Effects");
                drawList->AddText(MainWindow::Font, DPI_SCALED(40), {origin.x - textSize.x, (origin.y - (halfSquare * .8f)) - textSize.y}, ImGui::GetColorU32(ImGuiCol_TextDisabled), g_menuSpinType == Document ? "Document Effects" : "Local Effects");
                textSize = ImGui::CalcTextSize(options[index].data());
                drawList->AddText(MainWindow::Font, DPI_SCALED(40), {origin.x - textSize.x, origin.y - textSize.y}, ImGui::GetColorU32(ImGuiCol_Text), options[index].data());
                ImGui::PopFont();
                g_currentSpin += Gamepad_Spin(Gamepad::LeftStick);
                StickMenu(g_currentSpin, options);
            }
            else if(g_validSpin)
            {
                switch(index)
                {
                    case 0:
                        printf("Cancel\n");
                        break;
                    case 1:
                        printf("Add New\n");
                        g_editingEffectName = "";
                        ImGui::OpenPopup("Edit Singer##Gamepad");
                        break;
                    default:
                        if(g_menuSpinType == Document)
                        {
                            g_editingEffectName = g_docEffectNames[index - 2];
                            ImGui::OpenPopup("Edit Singer##Gamepad");
                        }
                        else
                        {
                            g_editingEffectName = g_localEffectNames[index - 2];
                            ImGui::OpenPopup("Edit Singer##Gamepad");
                        }
                        break;
                }
                g_validSpin = false;
                g_currentSpin = 0;
            }
        }
    }
}

void DoEffectsActions()
{
    if(g_layerLastFrame == Effects)
    {
        Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
        if(Gamepad_Tap(Gamepad::Triangle))
        {
            if(doc.GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue.starts_with("image "))
            {
                std::string timeStr = StringTools::Split(doc.GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue, " ")[1];
                g_imageFadeTime = (int)(std::stof(timeStr) * 100);
                g_editingEffectName = doc.GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue.substr(("image " + timeStr + " ").size());
                g_imageShiftTime = 0;
                ImGui::OpenPopup("Image##Gamepad");
            }
            else if(doc.IsEffectToken(doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken())))
            {
                History::AddRecord(new Serialization::LineRecord(History::Record::Edit, TimingEditor::Get().GetMarkedLine()), true);
                doc.GetLine(TimingEditor::Get().GetMarkedLine()).erase(doc.GetLine(TimingEditor::Get().GetMarkedLine()).begin() + TimingEditor::Get().GetMarkedToken());
                //std::vector<std::string> tags = StringTools::Split(doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken()).myValue.data(), std::regex("<[A-Za-z0-9#\"= -]+>"), true);
                //for(int i = 0; i < tags.size(); i++)
                //{
                //    if(tags[i].empty()) continue;
                //    std::string possibleAlias = tags[i];
                //    StringTools::EraseSubString(possibleAlias, "<");
                //    StringTools::EraseSubString(possibleAlias, ">");
                //    if(doc.GetEffectAliases().contains(possibleAlias))
                //    {
                //        g_menuSpinType = Document;
                //        g_editingEffectName = possibleAlias;
                //        ImGui::OpenPopup("Edit Singer##Gamepad");
                //    }
                //}
            }
            else if(g_lastAddedEffect != -1)
            {
                History::AddRecord(new Serialization::LineRecord(History::Record::Edit, TimingEditor::Get().GetMarkedLine()), true);
                Serialization::KaraokeToken& token = doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken());
                doc.GetLine(TimingEditor::Get().GetMarkedLine()).insert(doc.GetLine(TimingEditor::Get().GetMarkedLine()).begin() + TimingEditor::Get().GetMarkedToken(),
                    {("<" + g_docEffectNames[g_lastAddedEffect] + ">").data(), true, token.myStartTime});
            }
        }
        else if(Gamepad_Hold(Gamepad::Triangle))
        {
            bool hasLineTag = false;
            bool hasNoEffectTag = false;
            if(doc.GetLine(TimingEditor::Get().GetMarkedLine()).size() > 0)
            {
                hasLineTag = doc.GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue.starts_with("<line");
            }
            if(doc.GetLine(TimingEditor::Get().GetMarkedLine()).size() > (hasLineTag ? 1 : 0))
            {
                hasNoEffectTag = doc.GetToken(TimingEditor::Get().GetMarkedLine(), (hasLineTag ? 1 : 0)).myValue.starts_with("<no effect>");
            }
            if(Gamepad::GetButtonDown(Gamepad::D_Up) || Gamepad::GetButtonDown(Gamepad::D_Down))
            {
                History::AddRecord(new Serialization::LineRecord(History::Record::Edit, TimingEditor::Get().GetMarkedLine()), true);
                if(hasLineTag)
                {
                    bool changed = false;
                    int lane = std::stoi(StringTools::Split(doc.GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue, std::regex("[-\\d]+"), true)[0]);
                    bool isNegative = lane < 0;
                    if(Gamepad::GetButtonDown(Gamepad::D_Up))
                    {
                        lane++;
                        changed = true;
                    }
                    if(Gamepad::GetButtonDown(Gamepad::D_Down))
                    {
                        lane--;
                        changed = true;
                    }
                    if(changed)
                    {
                        int lanesShown = doc.GetFontSize() <= 43 ? 7 : doc.GetFontSize() <= 50 ? 6 : 5;
                        if(lane <= -lanesShown)
                        {
                            lane = lanesShown - 1;
                        }
                        if(lane >= lanesShown)
                        {
                            lane = -(lanesShown - 1);
                        }
                        if(lane == 0)
                        {
                            lane = isNegative ? 1 : -1;
                        }
                        doc.GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue = "<line#" + std::to_string(lane) + ">";
                        doc.MakeDirty();
                    }
                }
                else
                {
                    doc.GetLine(TimingEditor::Get().GetMarkedLine()).insert(doc.GetLine(TimingEditor::Get().GetMarkedLine()).begin(), {Gamepad::GetButtonDown(Gamepad::D_Up) ? "<line#1>" : "<line#-1>", false, 0});
                }
            }
            if(Gamepad::GetButtonDown(Gamepad::Circle))
            {
                History::AddRecord(new Serialization::LineRecord(History::Record::Edit, TimingEditor::Get().GetMarkedLine()), true);
                if(hasNoEffectTag)
                {
                    doc.GetLine(TimingEditor::Get().GetMarkedLine()).erase(doc.GetLine(TimingEditor::Get().GetMarkedLine()).begin() + (hasLineTag ? 1 : 0));
                }
                else
                {
                    doc.GetLine(TimingEditor::Get().GetMarkedLine()).insert(doc.GetLine(TimingEditor::Get().GetMarkedLine()).begin() + (hasLineTag ? 1 : 0), {"<no effect>", false, 0});
                }
            }
            if(Gamepad::GetButtonDown(Gamepad::LeftStick))
            {
                g_effectSpinType = (!PreviewWindow::GetHasVideo() && PreviewWindow::GetBackgroundElementPaths().size() > 1) ? (EffectSpinType)!g_effectSpinType : Singer;
            }
            //bool hasRemoveOption = false;
            // Force menu when on image
            if(doc.GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue.starts_with("image "))
            {
                g_effectSpinType = Image;
                //hasRemoveOption = true;
            }
            //else if(doc.IsEffectToken(doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken())))
            //{
            //    hasRemoveOption = true;
            //}
            float mag = Gamepad_Magnitude(Gamepad::LeftStick);
            std::vector<std::string> options = g_effectSpinType == Singer ? g_docEffectNames : PreviewWindow::GetBackgroundElementPaths();
            //if(hasRemoveOption) { options.emplace(options.begin(), "Remove"); }
            options.emplace(options.begin(), "Cancel");
            float adjustedSpin = g_currentSpin + .5f;
            while(adjustedSpin < 0) { adjustedSpin += options.size();}
            int index = ((int)adjustedSpin) % options.size();
            if(mag > .8f)
            {
                if(!g_validSpin) { SetUpDocumentEffectData(); }
                ImDrawList* drawList = ImGui::GetForegroundDrawList();
                ImVec2 origin = {((float)MainWindow::SwapWidth) * .5f, ((float)MainWindow::SwapHeight) * .5f};
                float halfSquare = origin.x < origin.y ? origin.x : origin.y;
                g_validSpin = true;
                ImGui::PushFont(MainWindow::Font);
                if(index == 0)
                {
                    // Cancel symbol
                }
                //else if(hasRemoveOption && index == 1)
                //{
                //    // Remove symbol
                //}
                else if(g_effectSpinType == Singer)
                {
                    Serialization::KaraokeColorEffect* effect = (Serialization::KaraokeColorEffect*)doc.GetEffectAliases().at(options[index]);
                    if(effect->myHasEndColor)
                    {
                        drawList->AddRectFilled(
                            {origin.x - (halfSquare * .3f), origin.y - (halfSquare * .3f)},
                            {origin.x - (halfSquare * .1f), origin.y - (halfSquare * .1f)},
                            IM_COL32_FROM_DOC(effect->myStartColor), DPI_SCALED(15));
                        drawList->AddRectFilled(
                            {origin.x + (halfSquare * .1f), origin.y - (halfSquare * .3f)},
                            {origin.x + (halfSquare * .3f), origin.y - (halfSquare * .1f)},
                            IM_COL32_FROM_DOC(effect->myEndColor), DPI_SCALED(15));
                    }
                    else
                    {
                        drawList->AddRectFilled(
                            {origin.x - (halfSquare * .1f), origin.y - (halfSquare * .3f)},
                            {origin.x + (halfSquare * .1f), origin.y - (halfSquare * .1f)},
                            IM_COL32_FROM_DOC(effect->myStartColor), DPI_SCALED(15));
                    }
                    ImVec2 textSize = ImGui::CalcTextSize(effect->myECHOValue.data());
                    drawList->AddText(MainWindow::Font, DPI_SCALED(30), {origin.x - (textSize.x * .75f), (origin.y * 1.1f) - (textSize.y * .75f)}, ImGui::GetColorU32(ImGuiCol_TextDisabled), effect->myECHOValue.data());
                }
                else
                {
                    drawList->AddImage(PreviewWindow::GetBackgroundTexture(options[index]).myID, {origin.x - (halfSquare * .3f), origin.y - (halfSquare * .6f)}, {origin.x + (halfSquare * .3f), origin.y - (halfSquare * 0.2625f)});
                }
                ImVec2 textSize = ImGui::CalcTextSize(options[index].data());
                drawList->AddText(MainWindow::Font, DPI_SCALED(40), {origin.x - textSize.x, origin.y - textSize.y}, ImGui::GetColorU32(ImGuiCol_Text), options[index].data());
                ImGui::PopFont();
                g_currentSpin += Gamepad_Spin(Gamepad::LeftStick);
                StickMenu(g_currentSpin, options);
            }
            else if(g_validSpin)
            {
                switch(index)
                {
                    case 0:
                        printf("Cancel\n");
                        break;
                    //case 1:
                    //    if(hasRemoveOption)
                    //    {
                    //        doc.RemoveLine(TimingEditor::Get().GetMarkedLine());
                    //        if(doc.GetLine(TimingEditor::Get().GetMarkedLine()).size() == 1 && doc.IsPauseToken(TimingEditor::Get().GetMarkedLine(), 0))
                    //        {
                    //            doc.RemoveLine(TimingEditor::Get().GetMarkedLine());
                    //        }
                    //        doc.MakeDirty();
                    //        printf("Remove\n");
                    //        break;
                    //    }
                    default:
                        if(g_effectSpinType == Singer)
                        {
                            //if(hasRemoveOption)
                            //{
                            //    doc.GetLine(TimingEditor::Get().GetMarkedLine()).erase(doc.GetLine(TimingEditor::Get().GetMarkedLine()).begin() + TimingEditor::Get().GetMarkedToken());
                            //}
                            //else
                            //{
                                History::AddRecord(new Serialization::LineRecord(History::Record::Edit, TimingEditor::Get().GetMarkedLine()), true);
                                Serialization::KaraokeToken& token = doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken());
                                doc.GetLine(TimingEditor::Get().GetMarkedLine()).insert(doc.GetLine(TimingEditor::Get().GetMarkedLine()).begin() + TimingEditor::Get().GetMarkedToken(),
                                    {("<" + options[index] + ">").data(), true, token.myStartTime});
                            //}
                        }
                        else
                        {
                            //if(hasRemoveOption)
                            //{
                            //    std::string timeStr = StringTools::Split(doc.GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue, " ")[1];
                            //    doc.GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue = "image " + timeStr + " " + PreviewWindow::GetBackgroundElementPaths()[index - 2];
                            //    printf("Replace with %s\n", PreviewWindow::GetBackgroundElementPaths()[index - 2].data());
                            //}
                            //else
                            //{
                                uint imgTime = doc.GetThisOrNextTimedToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken()).myStartTime;
                                for(int line = TimingEditor::Get().GetMarkedLine(); line < doc.GetData().size(); line++)
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
                                        History::AddRecord(new Serialization::LineRecord(History::Record::Insert, line));
                                        History::AddRecord(new Serialization::LineRecord(History::Record::Insert, line), true);
                                        Serialization::KaraokeToken newToken = {};
                                        newToken.myValue = "";
                                        newToken.myHasStart = true;
                                        newToken.myStartTime = imgTime;
                                        doc.GetData().insert(doc.GetData().begin() + line, {newToken});
                                        newToken.myValue = "image 0.2 " + PreviewWindow::GetBackgroundElementPaths()[index - 1];
                                        newToken.myHasStart = false;
                                        newToken.myStartTime = 0;
                                        doc.GetData().insert(doc.GetData().begin() + line, {newToken});
                                        break;
                                    }
                                }
                            //}
                        }
                        doc.MakeDirty();
                        break;
                }
                g_validSpin = false;
                g_currentSpin = 0;
            }
        }
    }
}

void DoLayoutActions()
{
    if(g_layerLastFrame == Layout)
    {
        if(Gamepad::GetButtonDown(Gamepad::D_Up)) Menu::Edit_MoveLineUp();
        if(Gamepad::GetButtonDown(Gamepad::D_Down)) Menu::Edit_MoveLineDown();
        if(Gamepad::GetButtonDown(Gamepad::D_Left)) Menu::Edit_MergeLineUp();
        if(Gamepad::GetButtonDown(Gamepad::D_Right)) Menu::Edit_MergeLineDown();
        if(Gamepad::GetButtonDown(Gamepad::Cross)) Menu::Edit_DuplicateLine();
        if(Gamepad::GetButtonDown(Gamepad::Circle)) Menu::Edit_InsertLinebreak();
        if(Gamepad::GetButtonDown(Gamepad::LeftStick)) Menu::Edit_RemoveLine();
        if(Gamepad_FlickAxis(Gamepad::LeftStickY, Gamepad_FlickUP)) { Menu::Edit_Majuscule(); }
        if(Gamepad_FlickAxis(Gamepad::LeftStickY, Gamepad_FlickDOWN)) { Menu::Edit_Minuscule(); }
        if(Gamepad_FlickAxis(Gamepad::LeftStickX, Gamepad_FlickLEFT)) { Menu::Edit_Capital(); }
        if(Gamepad_FlickAxis(Gamepad::LeftStickX, Gamepad_FlickRIGHT)) { Menu::Edit_ToggleCase(); }
    }
}

int StickMenu(float aScroll, std::vector<std::string> someLabels)
{
    if(g_menuBGTexture.myID == 0)
    {
        ImGui::Ext::LoadImage("##MenuBGTexture", "GamepadSprites/MenuBG.png");
        ImGui::Ext::RenderTexture("##MenuBGTexture", g_menuBGTexture);
    }
    ImVec2 origin = {((float)MainWindow::SwapWidth) * .5f, ((float)MainWindow::SwapHeight) * .6f};
    float halfSquare = origin.x < origin.y ? origin.x : origin.y;
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    int index = (int)aScroll;
    float animStep = 0.6283185307f;
    float animPos = (aScroll - index) * -animStep;
    //drawList->PushTextureID(g_menuBGTexture.myID);
    //drawList->PathEllipticalArcTo({origin.x, origin.y}, halfSquare * .6f, halfSquare * .23f, 0, 3.f, .14159f);
    //drawList->PathStroke(IM_COL32_WHITE, 0, (int)(halfSquare * .2f));
    //drawList->PopTextureID();
    drawList->AddImage(g_menuBGTexture.myID, {origin.x - halfSquare, origin.y - (halfSquare * .25f)}, {origin.x + halfSquare, origin.y + (halfSquare * .75f)}, {0, 0}, {1, .5f}, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
    ImGui::PushFont(MainWindow::Font);
    while(index < 5) { index += someLabels.size();}
    for(int i = 0; i < 10; i++)
    {
        int drawInd = ((index - 5) + i) % someLabels.size();
        float scale = -cos(((float)i * animStep) + animPos);
        ImVec2 pos = {origin.x + (halfSquare * .6f * sin(((float)i * animStep) + animPos)), origin.y + (halfSquare * .23f * -cos(((float)i * animStep) + animPos))};
        ImVec2 textSize = ImGui::CalcTextSize(someLabels[drawInd].data());
        ImVec2 size = {textSize.x * .75f * scale, textSize.y * .75f * scale};
        drawList->AddEllipseFilled(pos, size.x * .55f, size.y * .55f, IM_COL32(105, 50, 105, (200 * scale)));
        drawList->AddText(MainWindow::Font, DPI_SCALED(30), {pos.x - size.x, pos.y - size.y}, IM_COL32(255, 255, 255, (255 * scale)), someLabels[drawInd].data());
    }
    ImGui::PopFont();
    drawList->AddImage(g_menuBGTexture.myID, {origin.x - halfSquare, origin.y - (halfSquare * .25f)}, {origin.x + halfSquare, origin.y + (halfSquare * .75f)}, {0, .5f}, {1, 1}, ImGui::GetColorU32(ImGuiCol_Border));
    //drawList->PathEllipticalArcTo({origin.x, origin.y - halfSquare * .1f}, halfSquare * .6f, halfSquare * .2f, 0, 3.f, .14159f);
    //drawList->PathStroke(IM_COL32_WHITE, 0, halfSquare * .02f);
    //drawList->PathEllipticalArcTo({origin.x, origin.y + halfSquare * .1f}, halfSquare * .6f, halfSquare * .25f, 0, .14159f, 3.f);
    //drawList->PathStroke(IM_COL32_WHITE, 0, halfSquare * .02f);
    return -1;
}

bool DrawImagePopup()
{
    if(ImGui::BeginPopupModal("Image##Gamepad"))
    {
        Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
        ImGui::SetWindowSize({DPI_SCALED(400), DPI_SCALED(300)}, ImGuiCond_Once);
        ImVec2 topLeft = ImGui::GetCursorPos();
        if(g_selectedColorSlider == 0)
        {
            if(Gamepad::GetButtonRepeating(Gamepad::D_Left) || Gamepad::GetButtonRepeating(Gamepad::D_Right))
            {
                g_imageShiftTime += Gamepad::GetButton(Gamepad::D_Right) ? 1 : -1;
            }
        }
        ImGui::Text("Shift Start Time (cs)");
        ImGui::SameLine();
        ImGui::DragInt("##Shift Start Time (cs)", &g_imageShiftTime);
        ImVec2 size = ImGui::GetItemRectSize();
        if(g_selectedColorSlider == 1)
        {
            topLeft = ImGui::GetCursorPos();
            if(Gamepad::GetButtonRepeating(Gamepad::D_Left) || Gamepad::GetButtonRepeating(Gamepad::D_Right))
            {
                g_imageFadeTime += Gamepad::GetButton(Gamepad::D_Right) ? 1 : -1;
            }
        }
        ImGui::Text("Fade Duration (cs)");
        ImGui::SameLine();
        ImGui::DragInt("##Fade Duration (cs)", &g_imageFadeTime);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        topLeft.x += ImGui::GetWindowPos().x;
        topLeft.y += ImGui::GetWindowPos().y;
        drawList->AddRect(
            {topLeft.x - DPI_SCALED(5), topLeft.y - DPI_SCALED(5)},
            {topLeft.x + size.x + DPI_SCALED(5), topLeft.y + size.y + DPI_SCALED(5)},
            ImGui::GetColorU32(ImGuiCol_NavHighlight), ImGui::GetStyle().FrameRounding + DPI_SCALED(2), 0, ImGui::GetStyle().WindowBorderSize);
        AddHudSpriteTo(drawList, HUDSprite::ArrowUpBtn, {(topLeft.x - DPI_SCALED(10)) + (size.x * .5f), topLeft.y - DPI_SCALED(20)}, {DPI_SCALED(20), DPI_SCALED(20)});
        AddHudSpriteTo(drawList, HUDSprite::ArrowDownBtn, {(topLeft.x - DPI_SCALED(10)) + (size.x * .5f), topLeft.y + size.y + DPI_SCALED(0)}, {DPI_SCALED(20), DPI_SCALED(20)});
        AddHudSpriteTo(drawList, HUDSprite::ArrowLeftBtn, {topLeft.x - DPI_SCALED(20), topLeft.y + (size.y * .5f) - DPI_SCALED(10)}, {DPI_SCALED(20), DPI_SCALED(20)});
        AddHudSpriteTo(drawList, HUDSprite::ArrowRightBtn, {topLeft.x + DPI_SCALED(0) + size.x, topLeft.y + (size.y * .5f) - DPI_SCALED(10)}, {DPI_SCALED(20), DPI_SCALED(20)});
        if(Gamepad::GetButtonDown(Gamepad::A) || Gamepad::GetButtonDown(Gamepad::Circle))
        {
            uint imgTime = 0;
            if(doc.GetLine(TimingEditor::Get().GetMarkedLine() + 1).size() == 1 && doc.IsPauseToken(TimingEditor::Get().GetMarkedLine() + 1, 0))
            {
                imgTime = doc.GetToken(TimingEditor::Get().GetMarkedLine() + 1, 0).myStartTime;
            }
            else
            {
                imgTime = doc.GetThisOrNextTimedToken(TimingEditor::Get().GetMarkedLine() + 1, 0).myStartTime;
                imgTime = imgTime < 200 ? 0 : (imgTime - 200);
            }
            if((Gamepad::GetButton(Gamepad::A) && g_imageShiftTime != 0) || Gamepad::GetButton(Gamepad::Triangle))
            {
                doc.RemoveLine(TimingEditor::Get().GetMarkedLine());
                if(doc.GetLine(TimingEditor::Get().GetMarkedLine()).size() == 1 && doc.IsPauseToken(TimingEditor::Get().GetMarkedLine(), 0))
                {
                    doc.RemoveLine(TimingEditor::Get().GetMarkedLine());
                }
            }
            if(Gamepad::GetButton(Gamepad::A))
            {
                if(g_imageShiftTime == 0)
                {
                    History::AddRecord(new Serialization::LineRecord(History::Record::Edit, TimingEditor::Get().GetMarkedLine()));
                    doc.GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue = "image " + std::to_string(((float)g_imageFadeTime * .01f)) + " " + g_editingEffectName;
                }
                else
                {
                    imgTime = (int)imgTime < -g_imageShiftTime ? 0 : (imgTime + g_imageShiftTime);
                    for(int line = 0; line < doc.GetData().size(); line++)
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
                            History::AddRecord(new Serialization::LineRecord(History::Record::Insert, line));
                            History::AddRecord(new Serialization::LineRecord(History::Record::Insert, line), true);
                            newToken.myValue = "";
                            newToken.myHasStart = true;
                            newToken.myStartTime = imgTime;
                            doc.GetData().insert(doc.GetData().begin() + line, {newToken});
                            newToken.myValue = "image " + std::to_string(((float)g_imageFadeTime * .01f)) + " " + g_editingEffectName;
                            newToken.myHasStart = false;
                            newToken.myStartTime = 0;
                            doc.GetData().insert(doc.GetData().begin() + line, {newToken});
                            doc.MakeDirty();
                            break;
                        }
                    }
                }
            }
            g_editingEffectName = "";
            g_imageFadeTime = 0;
            g_imageShiftTime = 0;
            History::ForceEndRecord();
            ImGui::CloseCurrentPopup();
        }
        if(Gamepad::GetButtonDown(Gamepad::D_Up))
        {
            g_selectedColorSlider--;
        }
        if(Gamepad::GetButtonDown(Gamepad::D_Down))
        {
            g_selectedColorSlider++;
        }
        g_selectedColorSlider = g_selectedColorSlider < 0 ? 0 : g_selectedColorSlider > 1 ? 1 : g_selectedColorSlider;

        Gamepad::Mapping conMap = Gamepad::GetMapping(Gamepad::GetControllerWithLastEvent());
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - (ImGui::GetTextLineHeightWithSpacing() + DPI_SCALED(5)));
        if(conMap <= Gamepad::PSClassic && conMap != Gamepad::Xinput)
        {
            ImGui::Text("(X) Apply   (O) Cancel   (/\\) + (O) Remove");
        }
        else
        {
            ImGui::Text("(A) Apply   (B) Cancel   (Y) + (B) Remove");
        }
        ImGui::EndPopup();
        return true;
    }
    return false;
}

bool DrawLatencyPopup()
{
    if(ImGui::BeginPopupModal("Latency##Gamepad"))
    {
        if(!g_hasInitMetronome)
        {
            g_hasInitMetronome = true;
            Settings::InitLatencyVisualization();
        }
        ImGui::SetWindowSize({DPI_SCALED(400), DPI_SCALED(300)}, ImGuiCond_Once);
        int latency = TimingEditor::Get().GetAudioLatencyOffset();
        ImGui::Image(g_hudTexture.myID, {ImGui::GetTextLineHeightWithSpacing(), ImGui::GetTextLineHeightWithSpacing()}, g_hudStartUVs[ArrowLeftBtn], g_hudEndUVs[ArrowLeftBtn]);
        ImGui::SameLine();
        if(ImGui::DragInt("##Latency", &latency))
        {
            TimingEditor::Get().SetAudioLatencyOffset(latency);
        }
        ImGui::SameLine();
        ImGui::Image(g_hudTexture.myID, {ImGui::GetTextLineHeightWithSpacing(), ImGui::GetTextLineHeightWithSpacing()}, g_hudStartUVs[ArrowRightBtn], g_hudEndUVs[ArrowRightBtn]);
        if(Gamepad_RepeatDelayed(Gamepad::D_Left, .1f, 1.5f))
        {
            TimingEditor::Get().SetAudioLatencyOffset(latency - (Gamepad::GetTimeSinceToggled(Gamepad::D_Left) < 3 ? 1 : 5));
        }
        if(Gamepad_RepeatDelayed(Gamepad::D_Right, .1f, 1.5f))
        {
            TimingEditor::Get().SetAudioLatencyOffset(latency + (Gamepad::GetTimeSinceToggled(Gamepad::D_Right) < 3 ? 1 : 5));
        }
        if(Gamepad::GetButtonDown(Gamepad::X) || Gamepad::GetButtonDown(Gamepad::Circle))
        {
            ImGui::CloseCurrentPopup();
        }
        Gamepad::Mapping conMap = Gamepad::GetMapping(Gamepad::GetControllerWithLastEvent());
        if(conMap <= Gamepad::PSClassic && conMap != Gamepad::Xinput)
        {
            ImGui::Text("Hit (X) when the bass hits.");
        }
        else
        {
            ImGui::Text("Hit (A) when the bass hits.");
        }

        // Visualization
        float sizeY = ImGui::GetWindowHeight() - ImGui::GetCursorPosY();
        float sizeX = ImGui::GetWindowWidth();
        float size = (sizeX < sizeY ? sizeX : sizeY) * .4f;
        int timeRaw = Settings::DrawLatencyVisualization({sizeX, sizeY});

        // Latency Detection
        if(Gamepad::GetButtonDown(Gamepad::Cross))
        {
            if(timeRaw < 150)
            {
                TimingEditor::Get().SetAudioLatencyOffset(timeRaw);
            }
            else
            {
                TimingEditor::Get().SetAudioLatencyOffset(timeRaw - 200);
            }
        }

        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - (ImGui::GetTextLineHeightWithSpacing() + DPI_SCALED(5)));
        if(conMap <= Gamepad::PSClassic && conMap != Gamepad::Xinput)
        {
            ImGui::Text("(O) Close");
        }
        else
        {
            ImGui::Text("(B) Close");
        }
        ImGui::EndPopup();
        return true;
    }
    else if(g_hasInitMetronome)
    {
        Settings::StopLatencyVisualization();
        g_hasInitMetronome = false;
    }
    return false;
}

bool DrawFontSizePopup()
{
    if(g_properties->DrawFontSizeGamepadPopup())
    {
        return true;
    }
    return false;
}

bool DrawDocColorsPopup()
{
    if(g_properties->DrawDefaultColorsGamepadPopup(g_selectedColorSlider))
    {
        if(Gamepad::GetButtonDown(Gamepad::D_Left))
        {
            g_selectedColorSlider--;
        }
        if(Gamepad::GetButtonDown(Gamepad::D_Right))
        {
            g_selectedColorSlider++;
        }
        g_selectedColorSlider = g_selectedColorSlider < 0 ? 0 : g_selectedColorSlider > 7 ? 7 : g_selectedColorSlider;
        return true;
    }
    return false;
}

bool DrawTimeShiftPopup()
{
    if(g_properties->DrawShiftTimingsGamepadPopup())
    {
        return true;
    }
    return false;
}

bool DrawSingerPopup()
{
    if(g_properties->DrawSingerColorsGamepadPopup(g_selectedColorSlider, g_menuSpinType == MenuSpinType::Local, g_editingEffectName))
    {
        if(Gamepad::GetButtonDown(Gamepad::D_Left))
        {
            g_selectedColorSlider--;
        }
        if(Gamepad::GetButtonDown(Gamepad::D_Right))
        {
            g_selectedColorSlider++;
        }
        g_selectedColorSlider = g_selectedColorSlider < 0 ? 0 : g_selectedColorSlider > 7 ? 7 : g_selectedColorSlider;
        return true;
    }
    return false;
}

void DrawOverlay()
{
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    //ImVec2 screenSize = {((float)MainWindow::SwapWidth) - DPI_SCALED(50), ((float)MainWindow::SwapHeight)};
    ImVec2 screenSize = ImGui::GetWindowSize();
    screenSize.x -= DPI_SCALED(50);
    ImVec2 drawSize = DPI_SCALED(400) < screenSize.x ? ImVec2{DPI_SCALED(400), DPI_SCALED(300)} : ImVec2{screenSize.x, screenSize.x * .75f};
    ImVec2 origin = {screenSize.x - drawSize.x, screenSize.y - drawSize.y};
    Gamepad::Mapping controllerMapping = Gamepad::GetMapping(Gamepad::GetControllerWithLastEvent());
    float magnitudeLStick = Gamepad_Magnitude(Gamepad::LeftStick);
    ImVec2 clampedLStick = magnitudeLStick < 1 ?
        ImVec2{Gamepad::GetAxisRaw(Gamepad::LeftStickX), Gamepad::GetAxisRaw(Gamepad::LeftStickY)} :
        ImVec2{Gamepad::GetAxisRaw(Gamepad::LeftStickX) / magnitudeLStick, Gamepad::GetAxisRaw(Gamepad::LeftStickY) / magnitudeLStick};

#define MapSwitch(Xin, PS, ProCon, JoyCon) (controllerMapping == Gamepad::Xinput ? (Xin) : (controllerMapping <= Gamepad::PSClassic ? (PS) : (controllerMapping == Gamepad::SwitchPro ? (ProCon) : (controllerMapping <= Gamepad::JoyConR ? (JoyCon) : (ProCon)))))
#define DrawHudSprite(sprite, posX, posY, sizeX, sizeY, col) drawList->AddImage(g_hudTexture.myID, {origin.x + (drawSize.x * posX), origin.y + (drawSize.x * posY)}, {origin.x + (drawSize.x * (sizeX + posX)), origin.y + (drawSize.x * (sizeY + posY))}, g_hudStartUVs[sprite], g_hudEndUVs[sprite], col)
#define DrawDPadUpScale(sprite, alpha, Y) DrawHudSprite(sprite, .1f, .015625f, .1f, Y, IM_COL32(alpha, alpha, 255, alpha))
#define DrawDPadLeftScale(sprite, alpha, X) DrawHudSprite(sprite, .015625f, .1f, X, .1f, IM_COL32(alpha, alpha, 255, alpha))
#define DrawDPadRightScale(sprite, alpha, X, flip) DrawHudSprite(sprite, ((flip ? .3f : .2f) - .015625f), .1f, X, .1f, IM_COL32(alpha, alpha, 255, alpha))
#define DrawDPadDownScale(sprite, alpha, Y, flip) DrawHudSprite(sprite, .1f, ((flip ? .3f : .2f) - .015625f), .1f, Y, IM_COL32(alpha, alpha, 255, alpha))
#define DrawDPadUp(sprite, alpha) DrawDPadUpScale(sprite, alpha, .1f)
#define DrawDPadLeft(sprite, alpha) DrawDPadLeftScale(sprite, alpha, .1f)
#define DrawDPadRight(sprite, alpha)DrawDPadRightScale(sprite, alpha, .1f, false)
#define DrawDPadDown(sprite, alpha) DrawDPadDownScale(sprite, alpha, .1f, false)
#define DrawFaceButtonEffect(sprite, alpha) MapSwitch(DrawHudSprite(sprite, .8f, .015625f, .1f, .1f, IM_COL32(255, 232, 35, alpha)), DrawHudSprite(sprite, .8f, .015625f, .1f, .1f, IM_COL32(35, 203, 211, alpha)), DrawHudSprite(sprite, .715625f, .1f, .1f, .1f, IM_COL32(alpha, alpha, 255, alpha)), DrawHudSprite(sprite, .715625f, .1f, .1f, .1f, IM_COL32(alpha, alpha, 255, alpha)))
#define DrawFaceButtonSetting(sprite, alpha) MapSwitch(DrawHudSprite(sprite, .715625f, .1f, .1f, .1f, IM_COL32(14, 82, 255, alpha)), DrawHudSprite(sprite, .715625f, .1f, .1f, .1f, IM_COL32(217, 173, 216, alpha)), DrawHudSprite(sprite, .8f, .015625f, .1f, .1f, IM_COL32(alpha, alpha, 255, alpha)), DrawHudSprite(sprite, .8f, .015625f, .1f, .1f, IM_COL32(alpha, alpha, 255, alpha)))
#define DrawFaceButtonDecline(sprite, alpha) MapSwitch(DrawHudSprite(sprite, (.9f - .015625f), .1f, .1f, .1f, IM_COL32(255, 19, 3, alpha)), DrawHudSprite(sprite, (.9f - .015625f), .1f, .1f, .1f, IM_COL32(246, 129, 132, alpha)), DrawHudSprite(sprite, .8f, (.2f - .015625f), .1f, .1f, IM_COL32(alpha, alpha, 255, alpha)), DrawHudSprite(sprite, .8f, (.2f - .015625f), .1f, .1f, IM_COL32(alpha, alpha, 255, alpha)))
#define DrawFaceButtonConfirm(sprite, alpha) MapSwitch(DrawHudSprite(sprite, .8f, (.2f - .015625f), .1f, .1f, IM_COL32(53, 217, 0, alpha)), DrawHudSprite(sprite, .8f, (.2f - .015625f), .1f, .1f, IM_COL32(152, 188, 228, alpha)), DrawHudSprite(sprite, (.9f - .015625f), .1f, .1f, .1f, IM_COL32(alpha, alpha, 255, alpha)), DrawHudSprite(sprite, (.9f - .015625f), .1f, .1f, .1f, IM_COL32(alpha, alpha, 255, alpha)))
#define DrawBumperLeft(sprite, alpha) DrawHudSprite(sprite, .35f, .06f, .1f, .1f, IM_COL32(alpha, alpha, 255, alpha))
#define DrawBumperRight(sprite, alpha) DrawHudSprite(sprite, .55f, .06f, .1f, .1f, IM_COL32(alpha, alpha, 255, alpha))
#define DrawTriggerLeftScale(sprite, alpha, Y) DrawHudSprite(sprite, .35f, (.05f - Y), .1f, Y, IM_COL32(alpha, alpha, 255, alpha))
#define DrawTriggerRightScale(sprite, alpha, Y) DrawHudSprite(sprite, .55f, (.05f - Y), .1f, Y, IM_COL32(alpha, alpha, 255, alpha))
#define DrawLStickUp(sprite, alpha) DrawHudSprite(sprite, .3f, (.25f - clamp(remap(clampedLStick.y, -1, 1, -.2f, .1f), .0f, .07f)), .1f, .1f, IM_COL32(alpha, alpha, 255, alpha))
#define DrawLStickRight(sprite, alpha) DrawHudSprite(sprite, (.2f + clamp(remap(clampedLStick.x, -1, 1, -.1f, .2f), -.07f, .0f)), .35f, .1f, .1f, IM_COL32(alpha, alpha, 255, alpha))
#define DrawLStickLeft(sprite, alpha) DrawHudSprite(sprite, (.4f + clamp(remap(clampedLStick.x, -1, 1, -.2f, .1f), .0f, .07f)), .35f, .1f, .1f, IM_COL32(alpha, alpha, 255, alpha))
#define DrawLStickDown(sprite, alpha) DrawHudSprite(sprite, .3f, (.45f - clamp(remap(clampedLStick.y, -1, 1, -.1f, .2f), -.07f, .0f)), .1f, .1f, IM_COL32(alpha, alpha, 255, alpha))
#define DrawLStickCenter(sprite, alpha) DrawHudSprite(sprite, (.3f + (clampedLStick.x * .1f)), (.35f - (clampedLStick.y * .1f)), .1f, .1f, IM_COL32(alpha, alpha, 255, alpha))

    DrawHudSprite(MapSwitch(DPadBGPS, DPadBGPS, DPadBG, BtnPadBG), .0f, .0f, .3f, .3f, IM_COL32_WHITE);
    DrawHudSprite(MapSwitch(BtnPadBG, BtnPadBGPS, BtnPadBG, BtnPadBG), .7f, .0f, .3f, .3f, IM_COL32_WHITE);
    DrawHudSprite((g_layerLastFrame == Settings || g_layerLastFrame == Effects) ? StickSpinBG : StickFlickBG, .25f, .3f, .2f, .2f, IM_COL32_WHITE);
    DrawBumperRight(BumperFill, 255);
    DrawTriggerRightScale(TriggerFill, 255, .15f);
    DrawBumperLeft(BumperFill, 255);
    DrawTriggerLeftScale(TriggerFill, 255, .15f);
    DrawLStickCenter(BtnFill, 255);
    //DrawHudSprite(StickSpinBG, .5f, .3f, .2f, .2f, IM_COL32_WHITE);   // Right stick might not need a spot
    if(Gamepad::GetButton(Gamepad::D_Up)) DrawDPadUpScale(MapSwitch(DPadFillPSY, DPadFillPSY, DPadFillY, BtnFill), 150, .15f);
    if(Gamepad::GetButton(Gamepad::D_Left)) DrawDPadLeftScale(MapSwitch(DPadFillPSX, DPadFillPSX, DPadFillX, BtnFill), 150, .15f);
    if(Gamepad::GetButton(Gamepad::D_Right)) DrawDPadRightScale(MapSwitch(DPadFillPSX, DPadFillPSX, DPadFillX, BtnFill), 150, -.15f, true);
    if(Gamepad::GetButton(Gamepad::D_Down)) DrawDPadDownScale(MapSwitch(DPadFillPSY, DPadFillPSY, DPadFillY, BtnFill), 150, -.15f, true);
    if(Gamepad::GetButton(Gamepad::Triangle)) DrawFaceButtonEffect(BtnFill, 150);
    if(Gamepad::GetButton(Gamepad::Square)) DrawFaceButtonSetting(BtnFill, 150);
    if(Gamepad::GetButton(Gamepad::Circle)) DrawFaceButtonDecline(BtnFill, 150);
    if(Gamepad::GetButton(Gamepad::Cross)) DrawFaceButtonConfirm(BtnFill, 150);
    if(Gamepad::GetButton(Gamepad::R1)) DrawBumperRight(BumperFill, 150);
    if(Gamepad::GetButton(Gamepad::R2)) DrawTriggerRightScale(TriggerFill, 150, .15f);
    if(Gamepad::GetButton(Gamepad::L1)) DrawBumperLeft(BumperFill, 150);
    if(Gamepad::GetButton(Gamepad::L2)) DrawTriggerLeftScale(TriggerFill, 150, .15f);
    if(Gamepad::GetButton(Gamepad::LeftStick)) DrawLStickCenter(BtnFill, 150);
    if(g_layerLastFrame == Standard || g_layerLastFrame == Adjust)
    {
        DrawFaceButtonDecline(TimeEnd, 255);
        DrawFaceButtonConfirm(g_layerLastFrame == Standard ? TimeStart : SyllableBtn, 255);
        DrawDPadUp(ArrowUpBtn, 255);
        DrawDPadLeft(ArrowLeftBtn, 255);
        DrawDPadRight(ArrowRightBtn, 255);
        DrawDPadDown(ArrowDownBtn, 255);
        DrawBumperRight(SpeedUp, 255);
        DrawTriggerRightScale(AdjustBtn, 255, .1f);
        DrawBumperLeft(SpeedDown, 255);
        DrawLStickUp(SeekToMarkBtn, 255);
        DrawLStickRight(FF5sBtn, 255);
        DrawLStickLeft(RW5sBtn, 255);
        DrawLStickDown((AudioPlayback::GetIsPlaying() ? PauseBtn : PlayBtn), 255);
        DrawLStickCenter(StopBtn, 255);
    }
    if(g_layerLastFrame == Standard)   // Only the ones that are unique to Standard
    {
        float effectX = MapSwitch(.8125f, .8125f, .625f, .625f);
        float effectY = MapSwitch(-.075f, -.075f, .1125f, .1125f);
        float menuX = MapSwitch(.625f, .625f, .8125f, .8125f);
        float menuY = MapSwitch(.1125f, .1125f, -.075f, -.075f);
        DrawFaceButtonEffect(EffectBtn, 255);
        if(Serialization::KaraokeDocument::Get().GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue.starts_with("image "))
        {
            DrawHudSprite(BtnFill, effectX, effectY, .075f, .075f, IM_COL32_WHITE);
            DrawHudSprite(ImageBtn, effectX, effectY, .075f, .075f, IM_COL32_WHITE);
        }
        else if(Serialization::KaraokeDocument::Get().IsEffectToken(Serialization::KaraokeDocument::Get().GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken())))
        {
            DrawHudSprite(BtnFill, effectX, effectY, .075f, .075f, IM_COL32_WHITE);
            DrawHudSprite(EffectBtn, effectX, effectY, .075f, .075f, IM_COL32_WHITE); // TODO: Switch out for a remove symbol. 
        }
        else if(g_lastAddedEffect != -1)
        {
            DrawHudSprite(BtnFill, effectX, effectY, .075f, .075f, IM_COL32_WHITE);
            DrawHudSprite(SingerBtn, effectX, effectY, .075f, .075f, IM_COL32_WHITE);
        }
        DrawFaceButtonSetting(MenuBtn, 255);
        DrawHudSprite(BtnFill, menuX, menuY, .075f, .075f, IM_COL32_WHITE);
        DrawHudSprite(MapSwitch(BtnPadBG, BtnPadBGPS, BtnPadBG, BtnPadBG), menuX, menuY, .075f, .075f, IM_COL32_WHITE);
        DrawTriggerLeftScale(LayoutBtn, 255, .1f);
    }
    if(g_layerLastFrame == Settings)
    {
        DrawFaceButtonSetting(MenuBtn, 255);
        DrawFaceButtonConfirm(PreviewBtn, 255);
        DrawDPadUp(Latency, 255);
        DrawDPadLeft(FontSize, 255);
        DrawDPadRight(DocColor, 255);
        DrawDPadDown(ShiftTimes, 255);
        DrawLStickUp((g_effectSpinType == Singer ? SingerBtn : ImageBtn), 255);
        DrawLStickCenter(MenuToggle, 255);
    }
    if(g_layerLastFrame == Effects)
    {
        DrawFaceButtonEffect(EffectBtn, 255);
        DrawFaceButtonDecline(NoEffectBtn, 255);
        DrawDPadUp(LineTagPlus, 255);
        DrawDPadDown(LineTagMinus, 255);
        DrawLStickUp(SingerBtn, 255);
        DrawLStickCenter(MenuToggle, 255);
    }
    if(g_layerLastFrame == Layout)
    {
        DrawFaceButtonConfirm(LineDuplicate, 255);
        DrawFaceButtonDecline(LineSplit, 255);
        DrawDPadUp(LineMoveUp, 255);
        DrawDPadLeft(LineMergeUp, 255);
        DrawDPadRight(LineMergeDown, 255);
        DrawDPadDown(LineMoveDown, 255);
        DrawTriggerLeftScale(LayoutBtn, 255, .1f);
        DrawLStickUp(CaseMajus, 255);
        DrawLStickRight(CaseToggle, 255);
        DrawLStickLeft(CaseCapital, 255);
        DrawLStickDown(CaseMinus, 255);
        DrawLStickCenter(LineRemove, 255);
    }
#undef MapSwitch
#undef DrawHudSprite
}