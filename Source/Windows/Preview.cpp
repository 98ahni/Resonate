#include "Preview.h"
#include <Extensions/imguiExt.h>
#include <Defines.h>
#include "AudioPlayback.h"
#include "TimingEditor.h"

PreviewWindow::PreviewWindow()
{
    ImGui::Ext::LoadImage("##testImage", "ResonateIconLarger.png");
    myTexture = 0;
    myLanes[0] = myBackLanes[0] = -1;
    myLanes[1] = myBackLanes[1] = -1;
    myLanes[2] = myBackLanes[2] = -1;
    myLanes[3] = myBackLanes[3] = -1;
    myLanes[4] = myBackLanes[4] = -1;
    myLanes[5] = myBackLanes[5] = -1;
    myLanes[6] = myBackLanes[6] = -1;
}

void PreviewWindow::OnImGuiDraw()
{
    Gui_Begin();
    ImVec2 windowSize = ImGui::GetWindowContentRegionMax();
    ImVec2 contentOffset = ImGui::GetWindowContentRegionMin();
    ImVec2 contentSize = {windowSize.x - contentOffset.x, windowSize.y - contentOffset.y};
    if(ImGui::Ext::RenderImage("##testImage", myTexture))
    {
        ImGui::Image(myTexture, contentSize);
    }

    ImGui::PushFont(ourFont);
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    float textScale = doc.GetFontSize() / 50;
    textScale *= (ImGui::GetTextLineHeightWithSpacing() * 6) / contentSize.y;
    ourFont->Scale = textScale;
    // ^^ Setup

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, DPI_SCALED(10)});
    for(int lane = 0; lane < 7; lane++)
    {
        for(int token = 0; token < doc.GetLine(myLanes[lane]).size(); token++)
        {
            uint start = doc.GetLine(myLanes[lane])[token].myHasStart ? doc.GetLine(myLanes[lane])[token].myStartTime : 0;
            uint end = doc.GetTokenAfter(myLanes[lane], token).myHasStart ? doc.GetTokenAfter(myLanes[lane], token).myStartTime : start;
            if(doc.GetToken(myLanes[lane], token).myValue.contains('<'))
            {
                doc.ParseEffectToken(doc.GetToken(myLanes[lane], token));
            }
            if(ImGui::Ext::TimedSyllable(doc.GetLine(myLanes[lane])[token].myValue, start, end, AudioPlayback::GetPlaybackProgress() - TimingEditor::Get().GetLatencyOffset(), false, true))
            {
            }
            ImGui::SameLine();
        }
        doc.PopColor();
        ImGui::NewLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - DPI_SCALED(3));
    }
    ImGui::PopStyleVar();

    // vv Reset
    ourFont->Scale = 1;
    ImGui::PopFont();
    Gui_End();
}

void PreviewWindow::SetFont(ImFont *aFont)
{
    ourFont = aFont;
}

int PreviewWindow::CalculateLanesNeeded(float aWidth)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    float textWidth = ImGui::CalcTextSize(doc.SerializeLineAsText(doc.GetLine(myNextDisplayLineIndex)).data()).x;
    return (int)(textWidth / (aWidth * ourFont->Scale)) + 1;
}

bool PreviewWindow::FillBackLanes(int aLaneCount)
{
    int nextLineNeeds;
    int foundPlace;
    for(int i = (aLaneCount / 2) + (nextLineNeeds / 2); i >= nextLineNeeds; i--)
    {
        foundPlace = i - nextLineNeeds;
        for(int j = 0; j < nextLineNeeds; j++)
        {
            if(myBackLanes[i - j] != -1)
            {
                foundPlace = -1;
            }
        }
        if(foundPlace != -1) break;
    }
    if(foundPlace == -1)
    {
        for(int i = (aLaneCount / 2) - (nextLineNeeds / 2); i < aLaneCount; i++)
        {
            foundPlace = i;
            for(int j = 0; j < nextLineNeeds; j++)
            {
                if(myBackLanes[i + j] != -1)
                {
                    foundPlace = -1;
                }
            }
            if(foundPlace != -1) break;
        }
    }
    if(foundPlace != -1)
    {
        for(int i = foundPlace; i < foundPlace + nextLineNeeds; i++)
        {
            myBackLanes[i] = myNextAddLineIndex;
        }
        myNextAddLineIndex++;
        return true;
    }
    return false;
}

bool PreviewWindow::TryDisplayLanes()
{
    int checkingLine = -1;
    int currentStartLane = -1;
    bool displayedNewLines = false;
    for(int i = 0; i < 7; i++)
    {
        if(checkingLine != myBackLanes[i])
        {
            for(int j = currentStartLane; currentStartLane != -1 && j < i; j++)
            {
                myLanes[j] = myBackLanes[j];
                myBackLanes[j] = -1;
                displayedNewLines = true;
            }
            checkingLine = myBackLanes[i];
            currentStartLane = i;
        }
        if(myLanes[i] != -1)
        {
            currentStartLane = -1;
        }
    }
    return displayedNewLines;
}
