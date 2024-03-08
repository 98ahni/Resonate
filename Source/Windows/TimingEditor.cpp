#include "TimingEditor.h"
#include <Serialization/KaraokeData.h>
#include <Extensions/imguiExt.h>
#include "AudioPlayback.h"

void TimingEditor::OnImGuiDraw()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    if(Gui_Begin())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 10});
        for(int line = 0; line < doc.GetData().size(); line++)
        {
            for(int token = 0; token < doc.GetLine(line).size(); token++)
            {
                if(myMarkedLine == line && myMarkedToken == token) DrawTextMarker();
                uint start = doc.GetLine(line)[token].myHasStart ? doc.GetLine(line)[token].myStartTime : 0;
                uint end = doc.GetTokenAfter(line, token).myHasStart ? doc.GetTokenAfter(line, token).myStartTime : start;
                if(ImGui::Ext::TimedSyllable(doc.GetLine(line)[token].myValue, start, end, AudioPlayback::GetPlaybackProgress(), doc.GetLine(line)[token].myHasStart))
                {
                    myMarkedLine = line;
                    myMarkedToken = token;
                }
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
        ImGui::PopStyleVar();
        if(ImGui::IsWindowFocused())
        {
            bool charMode = false;
            bool stepMarker = false;
            bool endPrevLine = false;
            if(ImGui::IsKeyDown(ImGuiKey_ModCtrl))
            {
                charMode = true;
            }
            if(ImGui::IsKeyPressed(ImGuiKey_Space, false))
            {
                if(charMode)
                {
                    // Merge/split token at mark
                    if(myMarkedChar != 0 || (myMarkedToken == 0 && !doc.GetToken(myMarkedLine, myMarkedToken).myHasStart))
                    {
                        doc.GetLine(myMarkedLine).insert(doc.GetLine(myMarkedLine).begin() + myMarkedToken + 1, 
                            {
                                doc.GetToken(myMarkedLine, myMarkedToken).myValue.substr(myMarkedChar),
                                true, 0
                            });
                        doc.GetToken(myMarkedLine, myMarkedToken).myValue = doc.GetToken(myMarkedLine, myMarkedToken).myValue.substr(0, myMarkedChar);
                    }
                    else
                    {
                        auto& lastToken = doc.GetTokenBefore(myMarkedLine, myMarkedToken);
                        if(Serialization::KaraokeDocument::IsNull(lastToken))
                        {
                            doc.GetToken(myMarkedLine, myMarkedToken).myHasStart = false;
                        }
                        else
                        {
                            lastToken.myValue += doc.GetToken(myMarkedLine, myMarkedToken).myValue;
                            doc.GetLine(myMarkedLine).erase(doc.GetLine(myMarkedLine).begin() + myMarkedToken);
                        }
                    }
                }
                else
                {
                    // TODO: check if space token or if first on line
                    if(myMarkedToken == 0)
                    {
                        endPrevLine = doc.GetToken(myMarkedLine, myMarkedToken).myStartTime == doc.GetTokenBefore(myMarkedLine, myMarkedToken).myStartTime; // Use GetTimedTokenBefore instead.
                    }
                    if(doc.IsPauseToken(myMarkedLine, myMarkedToken))
                    {
                        doc.GetTimedTokenAfter(myMarkedLine, myMarkedToken).myStartTime = AudioPlayback::GetPlaybackProgress();
                    }
                    else
                    {
                        doc.GetToken(myMarkedLine, myMarkedToken).myStartTime = AudioPlayback::GetPlaybackProgress();
                    }
                    stepMarker = true;
                }
            }
            if(ImGui::IsKeyPressed(ImGuiKey_Enter, false) || endPrevLine)
            {
                Serialization::KaraokeToken& prevToken = doc.GetTokenBefore(myMarkedLine, myMarkedToken); // Use GetTimedTokenBefore instead.
                // Space token already exists
                if(doc.IsPauseToken(prevToken)) prevToken.myStartTime = AudioPlayback::GetPlaybackProgress();
                // Token is on same line
                else if(prevToken.myHasStart && myMarkedToken != 0) doc.GetLine(myMarkedLine).insert(doc.GetLine(myMarkedLine).begin() + myMarkedToken, {"", true, AudioPlayback::GetPlaybackProgress()});
                // Token is on previous line
                else doc.GetValidLineBefore(myMarkedLine).push_back({"", true, AudioPlayback::GetPlaybackProgress()});
            }
            if(ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            {
                myMarkedChar = 0;
                myMarkedLine = myMarkedLine > 0 ? myMarkedLine - 1 : 0;
                myMarkedToken = myMarkedToken < doc.GetLine(myMarkedLine).size() ? myMarkedToken : (doc.GetLine(myMarkedLine).size() - 1);
            }
            if(ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            {
                myMarkedChar = 0;
                myMarkedLine = myMarkedLine + 1 < doc.GetData().size() ? myMarkedLine + 1 : (doc.GetData().size() - 1);
                myMarkedToken = myMarkedToken < doc.GetLine(myMarkedLine).size() ? myMarkedToken : (doc.GetLine(myMarkedLine).size() - 1);
            }
            else
            {
                if(ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
                {
                    myMarkedChar = charMode ? myMarkedChar - 1 : 0;
                    if(myMarkedChar < 0 || !charMode)
                    {
                        myMarkedToken -= doc.IsPauseToken(myMarkedLine, myMarkedToken) ? 2 : 1;
                        if(myMarkedToken < 0)
                        {
                            if(myMarkedLine <= 0)
                            {
                                myMarkedToken = 0;
                                charMode = false;
                            }
                            else
                            {
                                myMarkedLine = myMarkedLine > 0 ? myMarkedLine - 1 : 0;
                                myMarkedToken = doc.GetLine(myMarkedLine).size() - 1;
                            }
                        }
                        myMarkedChar = charMode ? doc.GetToken(myMarkedLine, myMarkedToken).myValue.size() - 1 : 0;
                    }
                }
                if(ImGui::IsKeyPressed(ImGuiKey_RightArrow) || stepMarker)
                {
                    myMarkedChar = charMode ? myMarkedChar + 1 : 0;
                    if(myMarkedChar >= doc.GetToken(myMarkedLine, myMarkedToken).myValue.size() || !charMode)
                    {
                        myMarkedToken += doc.IsPauseToken(myMarkedLine, myMarkedToken) ? 2 : 1;
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
            }
        }
    }
    Gui_End();
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
