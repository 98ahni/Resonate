#include "TimingEditor.h"
#include <Serialization/KaraokeData.h>
#include <Extensions/imguiExt.h>
#include "AudioPlayback.h"

void TimingEditor::OnImGuiDraw()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    if(ImGui::Begin(GetName().c_str()))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 10});
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
                if(ImGui::Ext::TimedSyllable(doc.GetLine(line)[token].myValue, start, end, AudioPlayback::GetPlaybackProgress(), doc.GetLine(line)[token].myHasStart))
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
                }
                ImGui::SameLine();
            }
            doc.PopColor();
            ImGui::NewLine();
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
            lastToken.myValue += doc.GetToken(myMarkedLine, myMarkedToken).myValue;
            doc.GetLine(myMarkedLine).erase(doc.GetLine(myMarkedLine).begin() + myMarkedToken);
            MoveMarkerLeft();
        }
    }
}

void TimingEditor::RecordStartTime()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    // TODO: check if space token or if first on line
    if(myMarkedToken == 0)
    {
        auto& prevToken = doc.GetTimedTokenBefore(myMarkedLine, myMarkedToken);
        if(!doc.IsPauseToken(prevToken) || doc.GetToken(myMarkedLine, myMarkedToken).myStartTime == prevToken.myStartTime)
        {
            RecordEndTime();
        }
        //else if(!doc.IsNull(prevToken))
        //{
            
        //}
    }
    if(doc.IsPauseToken(myMarkedLine, myMarkedToken))
    {
        doc.GetTimedTokenAfter(myMarkedLine, myMarkedToken).myStartTime = AudioPlayback::GetPlaybackProgress();
    }
    else
    {
        doc.GetToken(myMarkedLine, myMarkedToken).myStartTime = AudioPlayback::GetPlaybackProgress();
    }
    MoveMarkerRight();
}

void TimingEditor::RecordEndTime()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    Serialization::KaraokeToken& prevToken = doc.GetTimedTokenBefore(myMarkedLine, myMarkedToken); // Use GetTimedTokenBefore instead.
    // Space token already exists
    if(doc.IsPauseToken(prevToken))
    {
        prevToken.myStartTime = AudioPlayback::GetPlaybackProgress();
        prevToken.myHasStart = true;
    }
    // Token is on same line
    else if(prevToken.myHasStart && myMarkedToken != 0)
    {
        doc.GetLine(myMarkedLine).insert(doc.GetLine(myMarkedLine).begin() + myMarkedToken, {"", true, AudioPlayback::GetPlaybackProgress()});
    }
    // Token is on previous line
    else
    {
        Serialization::KaraokeLine& prevLine = doc.GetValidLineBefore(myMarkedLine);
        if(!doc.IsNull(prevLine))
        {
            prevLine.push_back({"", true, AudioPlayback::GetPlaybackProgress()});
        }
    }
}

void TimingEditor::MoveMarkerUp()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    myMarkedChar = 0;
    myMarkedLine = myMarkedLine > 0 ? myMarkedLine - 1 : 0;
    myMarkedToken = myMarkedToken < doc.GetLine(myMarkedLine).size() ? myMarkedToken : (doc.GetLine(myMarkedLine).size() - 1);
}

void TimingEditor::MoveMarkerDown()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    myMarkedChar = 0;
    myMarkedLine = myMarkedLine + 1 < doc.GetData().size() ? myMarkedLine + 1 : (doc.GetData().size() - 1);
    myMarkedToken = myMarkedToken < doc.GetLine(myMarkedLine).size() ? myMarkedToken : (doc.GetLine(myMarkedLine).size() - 1);
}

void TimingEditor::MoveMarkerLeft(bool aIsCharmode)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    myMarkedChar = aIsCharmode ? myMarkedChar - 1 : 0;
    if(myMarkedChar < 0 || !aIsCharmode)
    {
        myMarkedToken -= doc.IsPauseToken(doc.GetTokenBefore(myMarkedLine, myMarkedToken)) ? 2 : 1;
        if(myMarkedToken < 0)
        {
            if(myMarkedLine <= 0)
            {
                myMarkedToken = 0;
                aIsCharmode = false;
            }
            else
            {
                myMarkedLine = myMarkedLine > 0 ? myMarkedLine - 1 : 0;
                myMarkedToken = doc.GetLine(myMarkedLine).size() - 1;
            }
        }
        myMarkedChar = aIsCharmode ? doc.GetToken(myMarkedLine, myMarkedToken).myValue.size() - 1 : 0;
    }
}

void TimingEditor::MoveMarkerRight(bool aIsCharmode)
{
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
}

void TimingEditor::SetInputUnsafe(bool anUnsafe)
{
    myInputIsUnsafe = anUnsafe;
    myDisableInput = anUnsafe ? true : myDisableInput;
}

void TimingEditor::DrawTextMarker()
{
    if(((int)(ImGui::GetTime() * 4)) % 2) return;
    ImVec2 endPos, startPos = endPos = ImGui::GetCursorScreenPos();
    startPos.y -= 2;
    endPos.y += 2 + ImGui::GetTextLineHeight();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddLine(startPos, endPos, IM_COL32_WHITE, 1);
    if(myMarkedChar != 0)
    {
        float offset = ImGui::CalcTextSize(Serialization::KaraokeDocument::Get().GetToken(myMarkedLine, myMarkedToken).myValue.substr(0, myMarkedChar).c_str()).x;
        startPos.x += offset;
        endPos.x += offset;
        drawList->AddLine(startPos, endPos, IM_COL32(255, 200, 200, 200), 1);
    }
}
