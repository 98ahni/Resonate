#include "Preview.h"
#include <Extensions/imguiExt.h>
#include <Defines.h>
#include "AudioPlayback.h"
#include "TimingEditor.h"

PreviewWindow::PreviewWindow()
{
    ImGui::Ext::LoadImage("##testImage", "ResonateIconLarger.png");
    myTexture = 0;
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
    uint playbackProgress = AudioPlayback::GetPlaybackProgress() - TimingEditor::Get().GetLatencyOffset();
    // ^^ Setup

    // I think it's only lane spacing and counting left!

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, DPI_SCALED(10)});
    for(int lane = 0; lane < 7; lane++)
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - DPI_SCALED(3));
        for(int token = myLanes[lane].myStartToken; token < myLanes[lane].myEndToken; token++)
        {
            if(!doc.GetToken(myLanes[lane].myLine, token).myHasStart)
            {
                continue;
            }
            uint start = doc.GetToken(myLanes[lane].myLine, token).myStartTime;
            uint end = doc.GetTimedTokenAfter(myLanes[lane].myLine, token).myStartTime;
            //if(doc.GetToken(myLanes[lane].myLine, token).myValue.contains('<'))
            if(doc.ParseEffectToken(doc.GetToken(myLanes[lane].myLine, token)))
            {
            }
            else if(ImGui::Ext::TimedSyllable(doc.GetToken(myLanes[lane].myLine, token).myValue, start, end, playbackProgress, false, true))
            {
            }
            ImGui::SameLine();
        }
        if(lane == 7 || myLanes[lane].myLine != myLanes[lane + 1].myLine)
        {
            doc.PopColor();
        }
    }
    ImGui::PopStyleVar();

    if(RemoveOldLanes(playbackProgress))
    {
        while(TryDisplayLanes())
        {
            while (FillBackLanes(7))
            {
            }
        }
    }

    // vv Reset
    ourFont->Scale = 1;
    ImGui::PopFont();
    Gui_End();
}

void PreviewWindow::SetFont(ImFont *aFont)
{
    ourFont = aFont;
}

int PreviewWindow::AssembleLanes(float aWidth)
{
    if(myAssemblyLanes[0].myLine == myNextAddLineIndex)
    {
        for(int lane = 0; lane < 7; lane++)
        {
            if(myAssemblyLanes[lane].myLine != myNextAddLineIndex)
            {
                return lane;
            }
        }
    }
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    int nextStartToken = 0;
    int lastSpaceToken = -1;
    for(int lane = 0; lane < 7; lane++)
    {
        if(doc.GetLine(myNextAddLineIndex).size() <= nextStartToken)
        {
            return lane;
        }
        myAssemblyLanes[lane].myLine = myNextAddLineIndex;
        myAssemblyLanes[lane].myStartToken = nextStartToken;
        float currentTextWidth = 0;
        do
        {
            if(doc.GetLine(myNextAddLineIndex).size() <= nextStartToken)
            {
                break;
            }
            currentTextWidth += ImGui::CalcTextSize(doc.GetToken(myNextAddLineIndex, nextStartToken).myValue.data()).x;
            if(doc.GetToken(myNextAddLineIndex, nextStartToken).myValue.ends_with(" "))
            {
                myAssemblyLanes[lane].myWidth = currentTextWidth;
                lastSpaceToken = nextStartToken;
            }
            nextStartToken++;
        } while(currentTextWidth < aWidth || lastSpaceToken == -1);
        myAssemblyLanes[lane].myEndToken = lastSpaceToken == -1 ? nextStartToken : lastSpaceToken;
        nextStartToken = lastSpaceToken;
        lastSpaceToken = -1;
    }
    return 7;
}

bool PreviewWindow::FillBackLanes(int aLaneCount)
{
    float width;
    int nextLineNeeds = AssembleLanes(width);
    int foundPlace;
    for(int i = (aLaneCount / 2) + (nextLineNeeds / 2); i >= nextLineNeeds; i--)
    {
        foundPlace = i - nextLineNeeds;
        for(int j = 0; j < nextLineNeeds; j++)
        {
            if(myBackLanes[i - j].myLine != -1)
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
                if(myBackLanes[i + j].myLine != -1)
                {
                    foundPlace = -1;
                }
            }
            if(foundPlace != -1) break;
        }
    }
    if(foundPlace != -1)
    {
        for(int i = 0; i < nextLineNeeds; i++)
        {
            myBackLanes[i + foundPlace] = myAssemblyLanes[i];
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
        if(checkingLine != myBackLanes[i].myLine)
        {
            for(int j = currentStartLane; currentStartLane != -1 && j < i; j++)
            {
                myLanes[j] = myBackLanes[j];
                myBackLanes[j].myLine = -1;
                displayedNewLines = true;
            }
            checkingLine = myBackLanes[i].myLine;
            currentStartLane = i;
        }
        if(myLanes[i].myLine != -1)
        {
            currentStartLane = -1;
        }
    }
    return displayedNewLines;
}

bool PreviewWindow::RemoveOldLanes(uint someCurrentTime)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    bool output = false;
    for(int lane = 0; lane < 7; lane++)
    {
        if(doc.GetLine(myLanes[lane].myLine).back().myStartTime < someCurrentTime)
        {
            myLanes[lane].myLine = -1;
            output = true;
        }
    }
    return output;
}
