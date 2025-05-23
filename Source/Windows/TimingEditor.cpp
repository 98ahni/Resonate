//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024-2025 98ahni> Original file author

#include "TimingEditor.h"
#include <Serialization/KaraokeData.h>
#include <Serialization/Preferences.h>
#include <Extensions/imguiExt.h>
#include "AudioPlayback.h"
#include "Preview.h"
#include "Console.h"
#include <Defines.h>
#include <StringTools.h>

TimingEditor& TimingEditor::Get()
{
    return *ourInstance;
}

TimingEditor::TimingEditor()
{
    ourInstance = this;
    myMarkedLine = 0;
    myMarkedToken = 0;
    myMarkedChar = 0;
    myMarkHasMoved = false;
    myInputIsUnsafe = false;
    myDisableInput = false;
    myLatencyOffset = 0;
    if(Serialization::Preferences::HasKey("Timing/Latency"))
    {
        myLatencyOffset = Serialization::Preferences::GetInt("Timing/Latency");
    }
}

void TimingEditor::OnImGuiDraw()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    if(ImGui::Begin(GetName().c_str(), 0, ImGuiWindowFlags_NoNavInputs | (Serialization::KaraokeDocument::Get().GetIsDirty() ? ImGuiWindowFlags_UnsavedDocument : 0)))
    {
        DrawImagePopup();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, DPI_SCALED(10)});
        bool useCustomFont = (myCustomFont != nullptr) && (!Serialization::Preferences::HasKey("Timing/CanUseCustomFont") || Serialization::Preferences::GetBool("Timing/CanUseCustomFont"));
        if(myFont) ImGui::PushFont(useCustomFont ? myCustomFont : myFont);
        for(int line = 0; line < doc.GetData().size(); line++)
        {
            Console::LineMargin(line);
            ImGui::SameLine();
            for(int token = 0; token < doc.GetLine(line).size(); token++)
            {
                if(!myDisableInput && myMarkedLine == line && myMarkedToken == token) DrawTextMarker();
                if(doc.GetToken(line, 0).myValue.starts_with("image "))
                {
                    DrawImageTagWidget(line, token);
                    Serialization::KaraokeLine& nextLine = doc.GetValidLineAfter(line);
                    if(nextLine.size() == 1 && doc.IsPauseToken(nextLine[0]))
                    {
                        line++;
                    }
                    ImGui::SameLine();
                    break;
                }
                uint start = doc.GetLine(line)[token].myHasStart ? doc.GetLine(line)[token].myStartTime : 0;
                uint end = doc.GetTokenAfter(line, token).myHasStart ? doc.GetTokenAfter(line, token).myStartTime : start;
                if(doc.GetToken(line, token).myValue.contains('<'))
                {
                    doc.ParseEffectToken(doc.GetToken(line, token));
                }
                if(doc.GetToken(line, token).myValue.starts_with("<line"))
                {
                    DrawLineTagWidget(line, token);
                }
                else if(ImGui::Ext::TimedSyllable(doc.GetLine(line)[token].myValue, start, end, AudioPlayback::GetPlaybackProgress() + myLatencyOffset, doc.GetLine(line)[token].myHasStart))
                {
                    if(ImGui::IsKeyDown(ImGuiKey_ModShift))
                    {
                        AudioPlayback::SetPlaybackProgress(doc.GetToken(line, token).myStartTime);
                    }
                    else
                    {
                        myMarkedLine = line;
                        myMarkedToken = token;
                        myMarkedChar = 0;
                    }
                    CheckMarkerIsSafe(true);
                }
                ImGui::SameLine();
            }
            doc.PopColor();
            ImGui::NewLine();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - DPI_SCALED(3));
        }
        if(myFont) ImGui::PopFont();
        ImGui::PopStyleVar();
        if(!myInputIsUnsafe && ImGui::IsWindowFocused())
        {
            myDisableInput = false;
        }
        if(!myDisableInput && !ImGui::IsKeyDown(ImGuiKey_ModShift) && !ImGui::IsKeyDown(ImGuiKey_ModAlt))
        {
            bool charMode = false;
            if(ImGui::IsKeyDown(ImGuiKey_ModCtrl))
            {
                charMode = true;
            }
            if(ImGui::IsKeyPressed(ImGuiKey_Space, false))
            {
                if(charMode)
                {
                    // Merge/split token at mark
                    ToggleTokenHasTime();
                }
                else
                {
                    RecordStartTime();
                }
            }
            if(ImGui::IsKeyPressed(ImGuiKey_Enter, false))
            {
                RecordEndTime();
            }
            if(ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            {
                MoveMarkerUp();
            }
            if(ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            {
                MoveMarkerDown();
            }
            if(ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
            {
                MoveMarkerLeft(charMode);
            }
            if(ImGui::IsKeyPressed(ImGuiKey_RightArrow))
            {
                MoveMarkerRight(charMode);
            }
        }
    }
    Gui_End();
    if(!myDisableInput && ImGui::IsKeyDown(ImGuiKey_ModShift) && !ImGui::IsKeyDown(ImGuiKey_ModCtrl) && !ImGui::IsKeyDown(ImGuiKey_ModAlt))
    {
        if(ImGui::IsKeyPressed(ImGuiKey_Space, false))
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
        if(ImGui::IsKeyPressed(ImGuiKey_Enter, false))
        {
            AudioPlayback::Stop();
        }
        if(ImGui::IsKeyPressed(ImGuiKey_UpArrow))
        {
            AudioPlayback::SetPlaybackSpeed(AudioPlayback::GetPlaybackSpeed() + 1);
        }
        if(ImGui::IsKeyPressed(ImGuiKey_DownArrow))
        {
            AudioPlayback::SetPlaybackSpeed(AudioPlayback::GetPlaybackSpeed() - 1);
        }
        if(ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
        {
            AudioPlayback::SetPlaybackProgress(AudioPlayback::GetPlaybackProgress() - (500.f * (AudioPlayback::GetPlaybackSpeed() * .1f)));
        }
        if(ImGui::IsKeyPressed(ImGuiKey_RightArrow))
        {
            AudioPlayback::SetPlaybackProgress(AudioPlayback::GetPlaybackProgress() + (500.f * (AudioPlayback::GetPlaybackSpeed() * .1f)));
        }
    }
}

void TimingEditor::SetFont(ImFont *aFont, ImFont* aCustomFont)
{
    myFont = aFont;
    myCustomFont = aCustomFont;
}

int TimingEditor::GetMarkedLine()
{
    return myMarkedLine;
}

int TimingEditor::GetMarkedToken()
{
    return myMarkedToken;
}

int TimingEditor::GetMarkedChar()
{
    return myMarkedChar;
}

void TimingEditor::ToggleTokenHasTime()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    doc.MakeDirty();
    History::AddRecord(new Serialization::LineRecord(History::Record::Edit, myMarkedLine));
    if(myMarkedChar != 0 || (myMarkedToken == 0 && !doc.GetToken(myMarkedLine, myMarkedToken).myHasStart))
    {
        doc.GetLine(myMarkedLine).insert(doc.GetLine(myMarkedLine).begin() + myMarkedToken + 1, 
            {
                doc.GetToken(myMarkedLine, myMarkedToken).myValue.substr(myMarkedChar),
                true, 0
            });
        doc.GetToken(myMarkedLine, myMarkedToken).myValue = doc.GetToken(myMarkedLine, myMarkedToken).myValue.substr(0, myMarkedChar);
        if(myMarkedToken != 0)
        {
            MoveMarkerRight();
        }
    }
    else
    {
        auto& lastToken = doc.GetTokenBefore(myMarkedLine, myMarkedToken);
        if(myMarkedToken == 0 || Serialization::KaraokeDocument::IsNull(lastToken))
        {
            doc.GetToken(myMarkedLine, myMarkedToken).myHasStart = false;
        }
        else
        {
            int lastTokenLength = lastToken.myValue.size();
            lastToken.myValue += doc.GetToken(myMarkedLine, myMarkedToken).myValue;
            doc.GetLine(myMarkedLine).erase(doc.GetLine(myMarkedLine).begin() + myMarkedToken);
            MoveMarkerLeft();
            myMarkedChar = lastTokenLength;
        }
    }
}

void TimingEditor::RecordStartTime()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    doc.MakeDirty();
    if(myMarkedToken == 0)
    {
        auto& prevToken = doc.GetTimedTokenBefore(myMarkedLine, myMarkedToken);
        if(!doc.IsPauseToken(prevToken) || doc.GetToken(myMarkedLine, myMarkedToken).myStartTime == prevToken.myStartTime)
        {
            RecordEndTime();
        }
    }
    History::AddRecord(new Serialization::LineRecord(History::Record::Edit, myMarkedLine));
    int scaledLatency = (float)myLatencyOffset * (float)AudioPlayback::GetPlaybackSpeed() * .1f;
    if(doc.IsPauseToken(myMarkedLine, myMarkedToken))
    {
        doc.GetTimedTokenAfter(myMarkedLine, myMarkedToken).myStartTime = AudioPlayback::GetPlaybackProgress() - scaledLatency;
    }
    else
    {
        doc.GetToken(myMarkedLine, myMarkedToken).myStartTime = AudioPlayback::GetPlaybackProgress() - scaledLatency;
    }
    MoveMarkerRight();
}

void TimingEditor::RecordEndTime()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    doc.MakeDirty();
    Serialization::KaraokeToken& currToken = doc.GetToken(myMarkedLine, myMarkedToken);
    Serialization::KaraokeToken& prevToken = doc.GetTimedTokenBefore(myMarkedLine, myMarkedToken);
    if(doc.IsNull(prevToken)) return;
    int scaledLatency = (float)myLatencyOffset * (float)AudioPlayback::GetPlaybackSpeed() * .1f;
    int currMarkLine = myMarkedLine;
    int currMarkToken = myMarkedToken;
    do
    {   // This is not how the marker system is meant to be used and I don't like it...
        MoveMarkerLeft();
        if(doc.GetToken(myMarkedLine, myMarkedToken).myHasStart)
        {
            History::AddRecord(new Serialization::LineRecord(History::Record::Edit, myMarkedLine));
            break;
        }
    } while (!doc.IsNull(doc.GetTimedTokenBefore(myMarkedLine, myMarkedToken)));
    myMarkedToken = currMarkToken;
    myMarkedLine = currMarkLine;
    // Space token already exists
    if(doc.IsPauseToken(prevToken))
    {
        prevToken.myStartTime = AudioPlayback::GetPlaybackProgress() - scaledLatency;
        prevToken.myHasStart = true;
    }
    // Space token already exists and marker is on it
    else if(doc.IsPauseToken(currToken))
    {
        currToken.myStartTime = AudioPlayback::GetPlaybackProgress() - scaledLatency;
        currToken.myHasStart = true;
        MoveMarkerRight();
    }
    // Token is on same line
    else if(prevToken.myHasStart && myMarkedToken != 0)
    {
        doc.GetLine(myMarkedLine).insert(doc.GetLine(myMarkedLine).begin() + myMarkedToken, {"", true, AudioPlayback::GetPlaybackProgress() - scaledLatency});
        MoveMarkerRight();
    }
    // Token is on previous line
    else
    {
        Serialization::KaraokeLine& prevLine = doc.GetValidLineBefore(myMarkedLine);
        if(!doc.IsNull(prevLine))
        {
            prevLine.push_back({"", true, AudioPlayback::GetPlaybackProgress() - scaledLatency});
        }
    }
}

void TimingEditor::MoveMarkerUp()
{
    myMarkHasMoved = true;
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    myMarkedChar = 0;
    myMarkedLine = myMarkedLine > 0 ? myMarkedLine - 1 : 0;
    //myMarkedToken = myMarkedToken < doc.GetLine(myMarkedLine).size() ? myMarkedToken : (doc.GetLine(myMarkedLine).size() - 1);
    CheckMarkerIsSafe(false);
}

void TimingEditor::MoveMarkerDown()
{
    myMarkHasMoved = true;
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    myMarkedChar = 0;
    myMarkedLine = myMarkedLine + 1 < doc.GetData().size() ? myMarkedLine + 1 : (doc.GetData().size() - 1);
    //myMarkedToken = myMarkedToken < doc.GetLine(myMarkedLine).size() ? myMarkedToken : (doc.GetLine(myMarkedLine).size() - 1);
    CheckMarkerIsSafe(true);
}

void TimingEditor::MoveMarkerLeft(bool aIsCharmode)
{
    myMarkHasMoved = true;
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    myMarkedChar = aIsCharmode ? myMarkedChar - 1 : 0;
    if(myMarkedChar < 0 || !aIsCharmode)
    {
        myMarkedToken -= doc.IsPauseToken(doc.GetTokenBefore(myMarkedLine, myMarkedToken)) ? 2 : 1;
        if(myMarkedToken < 0)
        {
            myMarkedToken = doc.GetValidLineBefore(myMarkedLine).size() - 1;
            myMarkedLine = myMarkedLine > 0 ? myMarkedLine - 1 : 0;
        }
        CheckMarkerIsSafe(false);
        myMarkedChar = aIsCharmode ? doc.GetToken(myMarkedLine, myMarkedToken).myValue.size() - 1 : 0;
    }
}

void TimingEditor::MoveMarkerRight(bool aIsCharmode)
{
    myMarkHasMoved = true;
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    myMarkedChar = aIsCharmode ? myMarkedChar + 1 : 0;
    if(myMarkedChar >= doc.GetToken(myMarkedLine, myMarkedToken).myValue.size() || !aIsCharmode)
    {
        myMarkedToken += doc.IsPauseToken(doc.GetTokenAfter(myMarkedLine, myMarkedToken)) ? 2 : 1;
        if(myMarkedToken >= doc.GetLine(myMarkedLine).size())
        {
            //myMarkedLine = myMarkedLine + 1 < doc.GetData().size() ? myMarkedLine + 1 : (doc.GetData().size() - 1);
            myMarkedLine = myMarkedLine + 1;
            if(doc.IsNull(doc.GetValidLineAfter(myMarkedLine)) && (myMarkedLine >= doc.GetData().size() || doc.GetLine(myMarkedLine).size() == 0))
            {
                myMarkedLine = (myMarkedLine >= doc.GetData().size() ? doc.GetData().size() : myMarkedLine) - 1;
                myMarkedToken = doc.GetLine(myMarkedLine).size() - 1;
                if(!doc.IsPauseToken(myMarkedLine, myMarkedToken))
                {
                    doc.GetLine(myMarkedLine).push_back({"", false, 0});
                    myMarkedToken += 1;
                }
            }
            else
            {
                myMarkedToken = 0;
            }
        }
        myMarkedChar = 0;
    }
    CheckMarkerIsSafe(true);
}

void TimingEditor::CheckMarkerIsSafe(bool aIsMovingRight)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    if(myMarkedLine < 0) myMarkedLine = 0;
    if(myMarkedLine >= doc.GetData().size()) myMarkedLine = doc.GetData().size() - 1;
    if(doc.GetLine(myMarkedLine).size() == 0)
    {
        // If the line is empty move to the next in specified direction if a valid target exists, else move back.
        if(aIsMovingRight)
        {
            !doc.IsNull(doc.GetValidLineAfter(myMarkedLine)) ? MoveMarkerDown() : MoveMarkerUp();
        }
        else
        {
            !doc.IsNull(doc.GetValidLineBefore(myMarkedLine)) ? MoveMarkerUp() : MoveMarkerDown();
        }
    }
    if(myMarkedToken < 0) myMarkedToken = 0;
    if(myMarkedToken >= doc.GetLine(myMarkedLine).size()) myMarkedToken = doc.GetLine(myMarkedLine).size() - 1;
    if(doc.IsPauseToken(myMarkedLine, myMarkedToken) && !doc.IsNull(doc.GetTokenAfter(myMarkedLine, myMarkedToken)))
    {
        if(aIsMovingRight)
        {
            myMarkedToken != doc.GetLine(myMarkedLine).size() - 1 || (doc.GetLine(myMarkedLine).size() == 1 && (myMarkedLine < doc.GetData().size() - 1)) ? MoveMarkerRight() : MoveMarkerLeft();
        }
        else// if(!aIsMovingRight && !doc.IsNull(doc.GetTokenBefore(myMarkedLine, myMarkedToken)))
        {
            MoveMarkerLeft();
        }
    }
}

void TimingEditor::SetInputUnsafe(bool anUnsafe)
{
    myInputIsUnsafe = anUnsafe;
    myDisableInput = anUnsafe ? true : myDisableInput;
}

bool TimingEditor::GetInputUnsafe()
{
    return myDisableInput;
}

void TimingEditor::SetLatencyOffset(int someCentiSeconds)
{
    myLatencyOffset = someCentiSeconds;
    Serialization::Preferences::SetInt("Timing/Latency", myLatencyOffset);
}

int TimingEditor::GetLatencyOffset()
{
    return myLatencyOffset;
}

void TimingEditor::DrawTextMarker()
{
    if(((int)(ImGui::GetTime() * 4)) % 2) return;
    ImVec2 endPos, startPos = endPos = ImGui::GetCursorScreenPos();
    startPos.y -= DPI_SCALED(2);
    endPos.y += DPI_SCALED(2) + ImGui::GetTextLineHeight();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddLine(startPos, endPos, IM_COL32_WHITE, 1);
    if(myMarkedChar != 0)
    {
        float offset = ImGui::CalcTextSize(Serialization::KaraokeDocument::Get().GetToken(myMarkedLine, myMarkedToken).myValue.substr(0, myMarkedChar).c_str()).x;
        startPos.x += offset;
        endPos.x += offset;
        drawList->AddLine(startPos, endPos, IM_COL32(255, 200, 200, 200), DPI_SCALED(1));
    }
    if(!myMarkHasMoved) return;
    myMarkHasMoved = false;
    if(endPos.x < ImGui::GetWindowPos().x)
    {
        //ImGui::SetScrollX(ImGui::GetCursorPos().x - 10);
        //ImGui::SetScrollFromPosX()
        ImGui::SetScrollHereX(0);
    }
    else if(endPos.y < ImGui::GetWindowPos().y)
    {
        ImGui::SetScrollHereY(0);
    }
    else if((ImGui::GetWindowPos().x + ImGui::GetWindowSize().x) < endPos.x)
    {
        ImGui::SetScrollHereX(1);
    }
    else if((ImGui::GetWindowPos().y + ImGui::GetWindowSize().y) < endPos.y)
    {
        ImGui::SetScrollHereY(1);
    }
}

void TimingEditor::DrawImagePopup()
{
    if(ImGui::BeginPopupModal("Edit Image", &myIsImagePopupOpen))
    {
        Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
        SetInputUnsafe(true);
        if(PreviewWindow::GetHasVideo()) {ImGui::BeginDisabled();}
        ImGui::BeginChild("##choose image", {0, DPI_SCALED(200)}, ImGuiChildFlags_Border);
        if(PreviewWindow::GetHasVideo())
        {
            ImGui::TextWrapped("\n\nImages are not available when a video is present in the project.\n");
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            for(const std::string& path : PreviewWindow::GetBackgroundElementPaths())
            {
                ImGui::BeginChild(("##" + path).data(), {0, 0}, ImGuiChildFlags_AutoResizeY | (myImagePopupSelectedPath == path ? ImGuiChildFlags_Border : 0));
                ImGui::Image(PreviewWindow::GetBackgroundTexture(path).myID, {ImGui::GetTextLineHeightWithSpacing() * 1.777777f /*(16/9)*/, ImGui::GetTextLineHeightWithSpacing()});
                ImGui::SameLine();
                ImGui::Text(path.data());
                ImGui::EndChild();
                if(ImGui::IsItemClicked())
                {
                    myImagePopupSelectedPath = path;
                }
            }
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        ImGui::Ext::StepInt("Shift Start Time (cs)", myImagePopupFadeStartShift, 1, 10);
        ImGui::Ext::StepInt("Fade Duration (cs)", myImagePopupFadeDuration, 1, 10);
        ImGui::PushStyleColor(ImGuiCol_Button, 0xFFFF0F2F);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0xFFFF1F5F);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xFFFF0F4F);
        if(PreviewWindow::GetHasVideo()) {ImGui::EndDisabled();}
        if(ImGui::Button("Remove"))
        {
            myIsImagePopupOpen = false;
            doc.RemoveLine(myImagePopupEditLine);
            if(doc.GetLine(myImagePopupEditLine).size() == 1 && doc.IsPauseToken(myImagePopupEditLine, 0))
            {
                doc.RemoveLine(myImagePopupEditLine);
            }
            doc.MakeDirty();
        }
        ImGui::SameLine();
        ImGui::PopStyleColor(3);
        if(!PreviewWindow::GetHasVideo() && ImGui::Button("Apply"))
        {
            myIsImagePopupOpen = false;
            uint imgTime = 0;
            if(doc.GetLine(myImagePopupEditLine + 1).size() == 1 && doc.IsPauseToken(myImagePopupEditLine + 1, 0))
            {
                imgTime = doc.GetToken(myImagePopupEditLine + 1, 0).myStartTime;
            }
            else
            {
                imgTime = doc.GetThisOrNextTimedToken(myImagePopupEditLine + 1, 0).myStartTime;
                imgTime = imgTime < 200 ? 0 : (imgTime - 200);
            }
            doc.RemoveLine(myImagePopupEditLine);
            if(doc.GetLine(myImagePopupEditLine).size() == 1 && doc.IsPauseToken(myImagePopupEditLine, 0))
            {
                doc.RemoveLine(myImagePopupEditLine);
            }
            imgTime = (int)imgTime < -myImagePopupFadeStartShift ? 0 : (imgTime + myImagePopupFadeStartShift);
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
                    newToken.myValue = "";
                    newToken.myHasStart = true;
                    newToken.myStartTime = imgTime;
                    doc.GetData().insert(doc.GetData().begin() + line, {newToken});
                    newToken.myValue = "image " + std::to_string(((float)myImagePopupFadeDuration * .01f)) + " " + myImagePopupSelectedPath;
                    newToken.myHasStart = false;
                    newToken.myStartTime = 0;
                    doc.GetData().insert(doc.GetData().begin() + line, {newToken});
                    doc.MakeDirty();
                    break;
                }
            }
        }
        ImGui::EndPopup();
        if(!myIsImagePopupOpen)
        {
            SetInputUnsafe(false);
        }
    }
}

void TimingEditor::DrawImageTagWidget(int aLine, int aToken)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    std::string timeStr = StringTools::Split(doc.GetToken(aLine, 0).myValue, " ")[1];
    std::string imgPath = doc.GetToken(aLine, 0).myValue.substr(("image " + timeStr + " ").size());
    ImExtTexture texture = PreviewWindow::GetBackgroundTexture(imgPath);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + DPI_SCALED(5));
    ImVec2 drawPos = ImGui::GetCursorPos();
    ImGui::SetCursorPosY(drawPos.y + DPI_SCALED(19));
    if(!myDisableInput && myMarkedLine == aLine && myMarkedToken == aToken) DrawTextMarker();
    uint imgTime = doc.GetTimedTokenAfter(aLine, 0).myStartTime;
    ImGui::Ext::TimedSyllable("<               >", imgTime, imgTime + (uint)(std::stof(timeStr) * 100), AudioPlayback::GetPlaybackProgress() - myLatencyOffset, true);
    ImGui::SetCursorPos(drawPos);
    ImGui::SetCursorPosX(ImGui::CalcTextSize("<").x + drawPos.x);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {DPI_SCALED(5), DPI_SCALED(4)});
    if(texture.myID != 0 && ImGui::ImageButton(("##" + std::to_string(aLine)).data(), texture.myID, {ImGui::GetTextLineHeightWithSpacing() * 1.777777f /*(16/9)*/, ImGui::GetTextLineHeightWithSpacing()}))
    {
        myImagePopupEditLine = aLine;
        myImagePopupSelectedPath = imgPath;
        myImagePopupFadeStartShift = 0;
        myImagePopupFadeDuration = (int)(std::stof(timeStr) * 100);
        myIsImagePopupOpen = true;
        ImGui::OpenPopup("Edit Image");
    }
    ImGui::PopStyleVar();
}

void TimingEditor::DrawLineTagWidget(int aLine, int aToken)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    float cursorY = ImGui::GetCursorPosY() + DPI_SCALED(5);
    ImGui::SetCursorPosY(cursorY);
    ImGui::Text("<line#");
    ImGui::SameLine();
    int lane = std::stoi(StringTools::Split(doc.GetToken(aLine, aToken).myValue, std::regex("[-\\d]+"), true)[0]);
    bool isNegative = lane < 0;
    bool changed = false;
    ImGui::SetNextItemWidth(DPI_SCALED(20));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {DPI_SCALED(5), 0});
    ImGui::SetCursorPosY(cursorY + DPI_SCALED(2));
    if(ImGui::DragInt(("##" + std::to_string(aLine)).data(), &lane, 1, 2))
    {
        changed = true;
    }
    ImGui::PopStyleVar();
    ImGui::SameLine();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(DPI_SCALED(-10), DPI_SCALED(-20)));
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(.55f, .47f));
    ImGui::SetCursorPosY(cursorY + DPI_SCALED(2));
    ImGui::BeginGroup();
    float height = DPI_SCALED(ImGui::GetTextLineHeightWithSpacing()) * .5f;
    if(ImGui::Button("+", {height, height}))
    {
        lane++;
        changed = true;
    }
    if(ImGui::Button("-", {height, height}))
    {
        lane--;
        changed = true;
    }
    ImGui::EndGroup();
    ImGui::PopStyleVar(3);
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
        doc.GetToken(aLine, aToken).myValue = "<line#" + std::to_string(lane) + ">";
        doc.MakeDirty();
    }
    ImGui::SameLine();
    ImGui::Text(">");
}
