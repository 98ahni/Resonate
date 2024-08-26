//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "TimingEditor.h"
#include <Serialization/KaraokeData.h>
#include <Serialization/Preferences.h>
#include <Extensions/imguiExt.h>
#include "AudioPlayback.h"
#include <Defines.h>

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
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, DPI_SCALED(10)});
        if(myFont) ImGui::PushFont(myFont);
        for(int line = 0; line < doc.GetData().size(); line++)
        {
            for(int token = 0; token < doc.GetLine(line).size(); token++)
            {
                if(!myDisableInput && myMarkedLine == line && myMarkedToken == token) DrawTextMarker();
                uint start = doc.GetLine(line)[token].myHasStart ? doc.GetLine(line)[token].myStartTime : 0;
                uint end = doc.GetTokenAfter(line, token).myHasStart ? doc.GetTokenAfter(line, token).myStartTime : start;
                if(doc.GetToken(line, token).myValue.contains('<'))
                {
                    doc.ParseEffectToken(doc.GetToken(line, token));
                }
                if(ImGui::Ext::TimedSyllable(doc.GetLine(line)[token].myValue, start, end, AudioPlayback::GetPlaybackProgress() - myLatencyOffset, doc.GetLine(line)[token].myHasStart))
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
        if(!myDisableInput)
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
}

void TimingEditor::SetFont(ImFont *aFont)
{
    myFont = aFont;
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
    if(doc.IsPauseToken(myMarkedLine, myMarkedToken))
    {
        doc.GetTimedTokenAfter(myMarkedLine, myMarkedToken).myStartTime = AudioPlayback::GetPlaybackProgress() + myLatencyOffset;
    }
    else
    {
        doc.GetToken(myMarkedLine, myMarkedToken).myStartTime = AudioPlayback::GetPlaybackProgress() + myLatencyOffset;
    }
    MoveMarkerRight();
}

void TimingEditor::RecordEndTime()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    doc.MakeDirty();
    Serialization::KaraokeToken& currToken = doc.GetToken(myMarkedLine, myMarkedToken);
    Serialization::KaraokeToken& prevToken = doc.GetTimedTokenBefore(myMarkedLine, myMarkedToken); // Use GetTimedTokenBefore instead.
    if(doc.IsNull(prevToken)) return;
    // Space token already exists
    if(doc.IsPauseToken(prevToken))
    {
        prevToken.myStartTime = AudioPlayback::GetPlaybackProgress() + myLatencyOffset;
        prevToken.myHasStart = true;
    }
    // Space token already exists and marker is on it
    else if(doc.IsPauseToken(currToken))
    {
        currToken.myStartTime = AudioPlayback::GetPlaybackProgress() + myLatencyOffset;
        currToken.myHasStart = true;
        MoveMarkerRight();
    }
    // Token is on same line
    else if(prevToken.myHasStart && myMarkedToken != 0)
    {
        doc.GetLine(myMarkedLine).insert(doc.GetLine(myMarkedLine).begin() + myMarkedToken, {"", true, AudioPlayback::GetPlaybackProgress() + myLatencyOffset});
        MoveMarkerRight();
    }
    // Token is on previous line
    else
    {
        Serialization::KaraokeLine& prevLine = doc.GetValidLineBefore(myMarkedLine);
        if(!doc.IsNull(prevLine))
        {
            prevLine.push_back({"", true, AudioPlayback::GetPlaybackProgress() + myLatencyOffset});
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
            if(myMarkedLine >= doc.GetData().size())
            {
                myMarkedLine = doc.GetData().size() - 1;
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
    if(doc.IsPauseToken(myMarkedLine, myMarkedToken))
    {
        if(aIsMovingRight)
        {
            myMarkedToken != doc.GetLine(myMarkedLine).size() - 1 ? MoveMarkerRight() : MoveMarkerLeft();
        }
        else
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

void TimingEditor::SetLatencyOffset(int someCentiSeconds)
{
    myLatencyOffset = someCentiSeconds;
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
