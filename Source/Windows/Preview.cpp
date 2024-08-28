#include "Preview.h"
#include <Extensions/imguiExt.h>
#include <Serialization/KaraokeData.h>
#include <Defines.h>
#include "MainWindow.h"
#include "AudioPlayback.h"
#include "TimingEditor.h"

PreviewWindow::PreviewWindow()
{
    ImGui::Ext::LoadImage("##testImage", "ResonateIconLarger.png");
    myTexture = 0;
    myNextAddLineIndex = 0;
    while (FillBackLanes(5, 1000))
    {
    }
    while(TryDisplayLanes())
    {
        while (FillBackLanes(5, 1000))
        {
        }
    }
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
    int lanesShown = doc.GetFontSize() <= 43 ? 7 : doc.GetFontSize() <= 50 ? 6 : 5;
    float textScale = doc.GetFontSize() / 50;
    textScale *= contentSize.y / ((50 + ImGui::GetStyle().ItemSpacing.y) * 6);
    ourFont->Scale = textScale == 0 ? .001f : textScale;
    uint playbackProgress = AudioPlayback::GetPlaybackProgress() - TimingEditor::Get().GetLatencyOffset();
    // ^^ Setup

	float laneHeight = ImGui::GetTextLineHeightWithSpacing();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, DPI_SCALED(10)});
    for(int lane = 0; lane < 7; lane++)
    {
        // TODO: Check if lane is two seconds from showing
        ImGui::SetCursorPosY((laneHeight * lane) + ImGui::GetStyle().ItemSpacing.y);
        ImGui::SetCursorPosX((contentSize.x - (myLanes[lane].myWidth * textScale)) * .5f);
        for(int token = myLanes[lane].myStartToken; token < myLanes[lane].myEndToken; token++)
        {
            if(!doc.GetToken(myLanes[lane].myLine, token).myHasStart)
            {
                continue;
            }
            uint start = doc.GetToken(myLanes[lane].myLine, token).myStartTime;
            uint end = doc.GetTimedTokenAfter(myLanes[lane].myLine, token).myStartTime;
            //if(doc.GetToken(myLanes[lane].myLine, token).myValue.contains('<'))
            if(!doc.ParseEffectToken(doc.GetToken(myLanes[lane].myLine, token)))
            {
                ImGui::Ext::TimedSyllable(doc.GetToken(myLanes[lane].myLine, token).myValue, start, end, playbackProgress, false, true);
                ImGui::SameLine();
            }
        }
        if(lane >= lanesShown) {break;}
        ImGui::NewLine();
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
            while (FillBackLanes(lanesShown, contentSize.x))
            {
            }
        }
    }

    // vv Reset
    ImGui::PopFont();
    Gui_End();
}

void PreviewWindow::SetFont(ImFont *aFont)
{
    ourFont = aFont;
}

int PreviewWindow::AssembleLanes(float aWidth)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
	if(doc.GetLine(myNextAddLineIndex).size() == 0) {return 0;}
	if(!doc.GetLine(myNextAddLineIndex).back().myHasStart) {return 0;}
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
    int nextStartToken = 0;
    int lastSpaceToken = -1;
    for(int lane = 0; lane < 7; lane++)
    {
        if(doc.GetLine(myNextAddLineIndex).size() <= nextStartToken)
        {
            printf("Line %i (size: %i) assembled and takes %i lanes\n", myNextAddLineIndex, doc.GetLine(myNextAddLineIndex).size(), lane);
            printf("\t%s\n", doc.SerializeLineAsText(doc.GetLine(myNextAddLineIndex)).data());
            return lane;
        }
        myAssemblyLanes[lane].myLine = myNextAddLineIndex;
        myAssemblyLanes[lane].myStartToken = nextStartToken;
        float currentTextWidth = 0;
        do
        {
            if(doc.GetLine(myNextAddLineIndex).size() <= nextStartToken)
            {
                myAssemblyLanes[lane].myWidth = currentTextWidth;
                lastSpaceToken = -1;
                break;
            }
            ImGui::PushFont(MainWindow::Font);
            currentTextWidth += ImGui::CalcTextSize(doc.GetToken(myNextAddLineIndex, nextStartToken).myValue.data()).x * 2.4f;
            ImGui::PopFont();
            if(doc.GetToken(myNextAddLineIndex, nextStartToken).myValue.ends_with(" "))
            {
                myAssemblyLanes[lane].myWidth = currentTextWidth;
                lastSpaceToken = nextStartToken;
            }
            nextStartToken++;
        } while(currentTextWidth < aWidth || lastSpaceToken == -1);
        myAssemblyLanes[lane].myEndToken = lastSpaceToken == -1 ? nextStartToken : lastSpaceToken;
        nextStartToken = lastSpaceToken == -1 ? nextStartToken : (lastSpaceToken + 1);
        lastSpaceToken = -1;
        printf("Lane %i has size %f\n", lane, myAssemblyLanes[lane].myWidth);
    }
    printf("Line %i (lenght: %i) assembled and takes all 7 lanes\n", myNextAddLineIndex, doc.GetLine(myNextAddLineIndex).size());
    printf("\t%s\n", doc.SerializeLineAsText(doc.GetLine(myNextAddLineIndex)).data());
    return 7;
}

bool PreviewWindow::FillBackLanes(int aLaneCount, float aScaledWidth)
{
    int nextLineNeeds = AssembleLanes(aScaledWidth);
	if(nextLineNeeds == 0)
    {
        myNextAddLineIndex++;
        return true;
    }
    int foundPlace = -1;
    for(int i = (aLaneCount / 2) + (nextLineNeeds / 2); i >= nextLineNeeds; i--)
    {
        foundPlace = i - nextLineNeeds;
        for(int j = 0; j < nextLineNeeds; j++)
        {
            if(myBackLanes[foundPlace - j].myLine != -1)
            {
                foundPlace = -1;
            }
        }
        if(foundPlace != -1) {break;}
    }
    if(foundPlace == -1)
    {
        for(int i = (aLaneCount / 2) - (nextLineNeeds / 2); i < aLaneCount; i++)
        {
            foundPlace = i;
            for(int j = 0; j < nextLineNeeds; j++)
            {
                if(myBackLanes[foundPlace + j].myLine != -1)
                {
                    foundPlace = -1;
                }
            }
            if(foundPlace != -1) {break;}
        }
    }
    if(foundPlace != -1)
    {
        for(int i = 0; i < nextLineNeeds; i++)
        {
            printf("Moving %i tokens from line %i to back lane %i\n", myAssemblyLanes[i].myEndToken - myAssemblyLanes[i].myStartToken, myAssemblyLanes[i].myLine, i + foundPlace);
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
    for(int lane = 0; lane < 7; lane++)
    {
        if(checkingLine != myBackLanes[lane].myLine)
        {
            for(int j = currentStartLane; currentStartLane != -1 && j < lane; j++)
            {
                printf("Moving line %i to display lane %i\n", myBackLanes[j].myLine, lane);
                myLanes[j] = myBackLanes[j];
                myBackLanes[j].myLine = -1;
                displayedNewLines = true;
            }
            checkingLine = myBackLanes[lane].myLine;
            currentStartLane = lane;
        }
        if(myLanes[lane].myLine != -1)
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
		if(myLanes[lane].myLine == -1 || doc.IsNull(doc.GetLine(myLanes[lane].myLine))) {continue;}
        if(doc.GetLine(myLanes[lane].myLine).back().myStartTime < someCurrentTime)
        {
            printf("Line %i is removed from lane %i\n", myLanes[lane].myLine, lane);
            myLanes[lane].myLine = -1;
            output = true;
        }
    }
    return output;
}
