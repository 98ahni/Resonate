//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "GamepadActions.h"
#include "Extensions/Gamepad.h"
#include "Extensions/imguiExt.h"
#include "Serialization/KaraokeData.h"
#include <Serialization/Preferences.h>
#include "Windows/MainWindow.h"
#include "Windows/TimingEditor.h"
#include "Windows/AudioPlayback.h"
#include "Windows/Preview.h"
#include "Defines.h"
#include "StringTools.h"
#include "EditMenu.h"

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
Serialization::KaraokeAliasMap g_localEffects = {};
std::vector<std::string> g_localEffectNames = {};
std::vector<std::string> g_docEffectNames = {};
int g_lastAddedEffect = -1;

void SetUpLocalEffectData()
{
    if(Serialization::Preferences::HasKey("StyleProperties/Keys"))
    {
        g_hasLocalEffects = true;
        g_localEffectNames.clear();
        std::vector<std::string> keys = StringTools::Split(Serialization::Preferences::GetString("StyleProperties/Keys"), ",");
        std::string uniqueKeys = "";
        for(std::string key : keys)
        {
            if(key == "" || g_localEffects.contains(key)) continue;
            uniqueKeys += uniqueKeys == "" ? key : ("," + key);
            g_localEffects[key] = Serialization::KaraokeDocument::ParseEffectProperty(Serialization::Preferences::GetString("StyleProperties/" + key));
            g_localEffectNames.push_back(key);
        }
        if(uniqueKeys != "") { Serialization::Preferences::SetString("StyleProperties/Keys", uniqueKeys); }
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

int StickMenu(float aScroll, std::vector<std::string> someLabels);

void DoGamepadActions()
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
    
    Edit (ZL)
    - D-pad Up/Down - Move line
    - D-pad Left/Right - Merge line up/down
    - [Confirm] - Duplicate line
    - [Decline] - Insert linebreak
    - L-stick click - RemoveLine
    - L-stick Flick Up - Majuscule case
    - L-stick Flick Down - Minuscule case
    - L-stick Flick Left - Capital case
    - L-stick Flick Right - Toggle letter case
    
    Char (ZR)
    - D-pad = arrows
    - [Confirm] - Split/Join
    */

    Layer layer = g_layerLastFrame;
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

    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    ImVec2 cursorResetPos = ImGui::GetCursorPos();
    if(layer == Standard || layer == Adjust)
    {
        if(Gamepad::GetButtonDown(Gamepad::D_Up)) TimingEditor::Get().MoveMarkerUp();
        if(Gamepad::GetButtonDown(Gamepad::D_Down)) TimingEditor::Get().MoveMarkerDown();
        if(Gamepad::GetButtonDown(Gamepad::D_Left)) TimingEditor::Get().MoveMarkerLeft(layer == Adjust);
        if(Gamepad::GetButtonDown(Gamepad::D_Right)) TimingEditor::Get().MoveMarkerRight(layer == Adjust);
        if(Gamepad::GetButtonDown(Gamepad::Circle)) TimingEditor::Get().RecordEndTime();
        if(Gamepad::GetButtonDown(Gamepad::Cross))
        {
            if(layer == Standard) TimingEditor::Get().RecordStartTime();
            else TimingEditor::Get().ToggleTokenHasTime();
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
    else if(layer == Settings)
    {
        if(Gamepad_Tap(Gamepad::Square))
        {
            // Show/Hide overlay
            printf("Toggle HUD\n");
        }
        else if(Gamepad_Hold(Gamepad::Square))
        {
            if(Gamepad::GetButtonDown(Gamepad::D_Up)) {}
            if(Gamepad::GetButtonDown(Gamepad::D_Down)) {}
            if(Gamepad::GetButtonDown(Gamepad::D_Left)) {}
            if(Gamepad::GetButtonDown(Gamepad::D_Right)) {}
            if(Gamepad::GetButtonDown(Gamepad::Cross)) {printf("Show Preview\n");}
            if(Gamepad::GetButtonDown(Gamepad::LeftStick)) {g_menuSpinType = (MenuSpinType)!g_menuSpinType;}
            float mag = Gamepad_Magnitude(Gamepad::LeftStick);
            std::vector<std::string> options = g_menuSpinType == Document ? g_docEffectNames : g_localEffectNames;
            options.emplace(options.begin(), "Add New");
            options.emplace(options.begin(), "Cancel");
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
                    Serialization::KaraokeColorEffect* effect = (Serialization::KaraokeColorEffect*)((g_menuSpinType == Document ? doc.GetEffectAliases() : g_localEffects).at(options[index]));
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
                        break;
                    default:
                        if(g_menuSpinType == Document) { printf("%s\n", g_docEffectNames[index - 2].data()); }
                        else                           { printf("%s\n", g_localEffectNames[index - 2].data()); }
                        break;
                }
                g_validSpin = false;
                g_currentSpin = 0;
            }
        }
    }
    else if(layer == Effects)
    {
        if(Gamepad_Tap(Gamepad::Triangle))
        {
            if(doc.GetToken(TimingEditor::Get().GetMarkedLine(), 0).myValue.starts_with("image "))
            {
                // Open image options
            }
            else if(doc.IsEffectToken(doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken())))
            {
                doc.GetLine(TimingEditor::Get().GetMarkedLine()).erase(doc.GetLine(TimingEditor::Get().GetMarkedLine()).begin() + TimingEditor::Get().GetMarkedToken());
            }
            else if(g_lastAddedEffect != -1)
            {
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
            float mag = Gamepad_Magnitude(Gamepad::LeftStick);
            std::vector<std::string> options = g_effectSpinType == Singer ? g_docEffectNames : PreviewWindow::GetBackgroundElementPaths();
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
                    default:
                        if(g_effectSpinType == Singer)
                        {
                            Serialization::KaraokeToken& token = doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken());
                            doc.GetLine(TimingEditor::Get().GetMarkedLine()).insert(doc.GetLine(TimingEditor::Get().GetMarkedLine()).begin() + TimingEditor::Get().GetMarkedToken(),
                                {("<" + g_docEffectNames[index - 1] + ">").data(), true, token.myStartTime});
                            printf("%s\n", g_docEffectNames[index - 1].data());
                        }
                        else { printf("%s\n", PreviewWindow::GetBackgroundElementPaths()[index - 1].data()); }
                        break;
                }
                g_validSpin = false;
                g_currentSpin = 0;
            }
        }
    }
    else if(layer == Layout)
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
    ImGui::SetCursorPos(cursorResetPos);
}

int StickMenu(float aScroll, std::vector<std::string> someLabels)
{
    ImVec2 origin = {((float)MainWindow::SwapWidth) * .5f, ((float)MainWindow::SwapHeight) * .6f};
    float halfSquare = origin.x < origin.y ? origin.x : origin.y;
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    int index = (int)aScroll;
    float animStep = 0.6283185307f;
    float animPos = (aScroll - index) * -animStep;
    drawList->PathEllipticalArcTo({origin.x, origin.y}, halfSquare * .6f, halfSquare * .23f, 0, 3.f, .14159f);
    drawList->PathStroke(IM_COL32(100, 20, 140, 50), 0, halfSquare * .2f);
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
    drawList->PathEllipticalArcTo({origin.x, origin.y - halfSquare * .1f}, halfSquare * .6f, halfSquare * .2f, 0, 3.f, .14159f);
    drawList->PathStroke(IM_COL32_WHITE, 0, halfSquare * .03f);
    drawList->PathEllipticalArcTo({origin.x, origin.y + halfSquare * .1f}, halfSquare * .6f, halfSquare * .25f, 0, .14159f, 3.f);
    drawList->PathStroke(IM_COL32_WHITE, 0, halfSquare * .03f);
    return -1;
}