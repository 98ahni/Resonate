//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024-2025 98ahni> Original file author

#include "Preview.h"
#include <emscripten.h>
#include <filesystem>
#include <StringTools.h>
#include <Extensions/FileHandler.h>
#include <Extensions/imguiExt.h>
#include <Serialization/KaraokeData.h>
#include <Serialization/Preferences.h>
#include <Defines.h>
#include "MainWindow.h"
#include "AudioPlayback.h"
#include "TimingEditor.h"
#include "Console.h"

extern"C" EMSCRIPTEN_KEEPALIVE void jsPlayPreviewVideo()
{
    ImGui::Ext::SetVideoSpeed("##PreviewBackground", AudioPlayback::GetPlaybackSpeed());
    ImGui::Ext::PlayVideo("##PreviewBackground");
}
extern"C" EMSCRIPTEN_KEEPALIVE void jsPausePreviewVideo()
{
    ImGui::Ext::PauseVideo("##PreviewBackground");
}
extern"C" EMSCRIPTEN_KEEPALIVE void jsSetPreviewVideoProgress()
{
    ImGui::Ext::SetVideoProgress("##PreviewBackground", AudioPlayback::GetPlaybackProgress() - TimingEditor::Get().GetVisualLatencyOffset());
    if(AudioPlayback::GetIsPlaying())
    {
        ImGui::Ext::PlayVideo("##PreviewBackground");
    }
    else
    {
        ImGui::Ext::PauseVideo("##PreviewBackground");
    }
}

PreviewWindow::PreviewWindow(bool anOnlyValidate)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    if(anOnlyValidate)
    {
        ourRulerFont->Scale = DPI_UNSCALED(((float)doc.GetFontSize() / 50.f));
        myPlaybackProgressLastFrame = 0;
        myNextAddLineIndex = 0;
        myShouldDebugDraw = false;
        Resetprogress();
        int lanesShown = doc.GetFontSize() <= 43 ? 7 : doc.GetFontSize() <= 50 ? 6 : 5;
        //for(int timer = 0; timer < 100000 && myNextAddLineIndex < doc.GetData().size(); timer += 10)
        //{
            while(RemoveOldLanes(INT_MAX, 50))
            {
                while(TryDisplayLanes())
                {
                    while (FillBackLanes(lanesShown))
                    {
                    }
                }
            }
        //}
        //Console::Log("Went through " + std::to_string(myNextAddLineIndex) + " lines out of a total of " + std::to_string(doc.GetData().size()) + ". ", myNextAddLineIndex);
        return;
    }
    ourHasVideo = false;
    std::string chosenBackground = "";
    bool allowVideo = !Serialization::Preferences::HasKey("Preview/LoadVideo") || Serialization::Preferences::GetBool("Preview/LoadVideo");
    for(auto& [path, tex] : ourBackgrounds)
    {
        std::filesystem::path fpath = path;
        if(fpath.extension() == ".mp4")
        {
            chosenBackground = allowVideo ? path : "../ResonateIconLarger.png";
            ourHasVideo = allowVideo;
            break;
        }
        if(doc.GetName() == fpath.filename().string())
        {
            chosenBackground = path;
        }
    }
    if(chosenBackground == "")
    {
        if(ourBackgrounds.size() != 0)
        {
            chosenBackground = ourBackgroundPaths[rand() % ourBackgroundPaths.size()];
        }
        else
        {
            chosenBackground = "../ResonateIconLarger.png";
        }
    }
    if(ourHasVideo)
    {
        ImGui::Ext::LoadVideo("##PreviewBackground", ("/local/" + chosenBackground).data());
        AudioPlayback::AddEventListener("play", "_jsPlayPreviewVideo");
        AudioPlayback::AddEventListener("pause", "_jsPausePreviewVideo");
        AudioPlayback::AddEventListener("seeked", "_jsSetPreviewVideoProgress");
        jsSetPreviewVideoProgress();
    }
    else
    {
        ImGui::Ext::LoadImage("##PreviewBackground", ("/local/" + chosenBackground).data());
    }
    ourRulerFont->Scale = DPI_UNSCALED(((float)doc.GetFontSize() / 50.f));
    myTexturePath = chosenBackground;
    myBackgroundQueue = std::deque<ImageFade>();
    myPlaybackProgressLastFrame = 0;
    myNextAddLineIndex = 0;
    myShouldDebugDraw = false;
    Resetprogress();
    if(!Serialization::Preferences::HasKey("Preview/UseOutline"))
    {
        Serialization::Preferences::SetBool("Preview/UseOutline", true);
    }
    ourTokenFlash = Serialization::Preferences::HasKey("Preview/TokenFlash") && Serialization::Preferences::GetBool("Preview/TokenFlash");
}

void PreviewWindow::OnImGuiDraw()
{
    ImGui::SetNextWindowSize({std::min(MainWindow::SwapWidth * .85f, DPI_SCALED(800.f)), std::min(MainWindow::SwapHeight * .8f, DPI_SCALED(450.f))}, ImGuiCond_Once);
    Gui_Begin();
    if(ourHasVideo && ImGui::Ext::IsVideoPaused("##PreviewBackground") && AudioPlayback::GetIsPlaying())
    {
        EM_ASM(audio_element_pause(););
    }
    else if(ourHasVideo && !ImGui::Ext::IsVideoPaused("##PreviewBackground") && !AudioPlayback::GetIsPlaying())
    {
        EM_ASM(audio_element_play(););
    }
    ImVec2 windowSize = ImGui::GetWindowContentRegionMax();
    ImVec2 contentOffset = ImGui::GetWindowContentRegionMin();
    ImVec2 contentSize = {windowSize.x - contentOffset.x, windowSize.y - contentOffset.y};
    ImVec2 aspect = {0.5625f, 1.777777f};
    if(contentSize.x < contentSize.y * aspect.y)
    {
        contentSize.y = contentSize.x * aspect.x;
        ImGui::SetCursorPosY((windowSize.y - contentSize.y) * .5f);
        contentOffset = ImGui::GetCursorPos();
    }
    else
    {
        contentSize.x = contentSize.y * aspect.y;
        ImGui::SetCursorPosX((windowSize.x - contentSize.x) * .5f);
        contentOffset = ImGui::GetCursorPos();
    }

    ImGui::PushFont(ourFont);
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    int lanesShown = doc.GetFontSize() <= 43 ? 7 : doc.GetFontSize() <= 50 ? 6 : 5;
    float fontScale = (float)doc.GetFontSize() / 50.f;
    float textScale = (DPI_UNSCALED(contentSize.y) / ((50 + DPI_UNSCALED(ImGui::GetStyle().ItemSpacing.y)) * 6));
    ourFont->Scale = fontScale * ((textScale < .001f ? .001f : textScale));
    uint playbackProgress = AudioPlayback::GetPlaybackProgress() - TimingEditor::Get().GetVisualLatencyOffset();
    if(((int)AudioPlayback::GetPlaybackProgress()) < TimingEditor::Get().GetVisualLatencyOffset())
    {
        playbackProgress = 0;
    }
    if(playbackProgress < myPlaybackProgressLastFrame)
    {
        Resetprogress();
    }
    myPlaybackProgressLastFrame = playbackProgress;
    // ^^ Setup

    ImGui::SetCursorPos(contentOffset);
    if(myBackgroundQueue.size() != 0 && playbackProgress > myBackgroundQueue.front().myEndTime)
    {
        myTexturePath = myBackgroundQueue.front().myImagePath;
        ImGui::Ext::LoadImage("##PreviewBackground", ("/local/" + myTexturePath).data());
        myBackgroundQueue.pop_front();
    }
    if(ImGui::Ext::RenderTexture("##PreviewBackground", ourBackgrounds[myTexturePath]))
    {
        ImGui::Image(ourBackgrounds[myTexturePath].myID, contentSize);
    }
    if(myBackgroundQueue.size() != 0)
    {
        ImGui::SetCursorPos(contentOffset);
        float start = myBackgroundQueue.front().myStartTime;
        float end = myBackgroundQueue.front().myEndTime;
        float alpha = remap(clamp(playbackProgress, start, end), start, end, 0.f, 1.f);
        ImGui::ImageWithBg(GetBackgroundTexture(myBackgroundQueue.front().myImagePath).myID, contentSize, {0, 0}, {1, 1}, {1, 1, 1, alpha});
    }

	//float lanePosY = ImGui::GetTextLineHeightWithSpacing();
	float lanePosY = contentSize.y / (float)lanesShown;
    float laneHeight = (lanePosY - ImGui::GetTextLineHeightWithSpacing()) * .5f;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, DPI_SCALED(10)});
    bool hasNoEffect = false;
    bool useOutline = Serialization::Preferences::GetBool("Preview/UseOutline");
    for(int lane = 0; lane < 7; lane++)
    {
        if(!CheckLaneVisible(lane, playbackProgress, 200)) {continue;}
        ImGui::SetCursorPosY((lanePosY * lane) + laneHeight + contentOffset.y);
        float cursorStartX = ((contentSize.x - (myLanes[lane].myWidth * DPI_SCALED(textScale))) * .5f) + contentOffset.x;
        ImGui::SetCursorPosX(cursorStartX);

        for(int token = myLanes[lane].myStartToken; token < myLanes[lane].myEndToken; token++)
        {
            uint start = doc.GetToken(myLanes[lane].myLine, token).myStartTime;
            uint end = doc.GetTimedTokenAfter(myLanes[lane].myLine, token).myStartTime;
            if(!doc.ParseEffectToken(doc.GetToken(myLanes[lane].myLine, token)))
            {
                ImGui::Ext::TimedSyllable(doc.GetToken(myLanes[lane].myLine, token).myValue, start, end, playbackProgress, false, ourTokenFlash, true, useOutline ? DPI_SCALED(2 * textScale) : 0, hasNoEffect ? 1 : 1.15f);
                ImGui::SameLine();
            }
            else if(doc.GetToken(myLanes[lane].myLine, token).myValue.starts_with("<no effect>"))
            {
                hasNoEffect = true;
            }
        }
        if(lane >= lanesShown) {break;}
        ImGui::NewLine();
        if(lane == 7 || myLanes[lane].myLine != myLanes[lane + 1].myLine)
        {
            doc.PopColor();
            hasNoEffect = false;
        }
    }
    ImGui::PopStyleVar();
    if(RemoveOldLanes(playbackProgress, 50))
    {
        while(TryDisplayLanes())
        {
            while (FillBackLanes(lanesShown))
            {
            }
        }
    }

    // vv Reset
    ImGui::PopFont();

    if(ImGui::IsKeyPressed(ImGuiKey_0, false))
    {
        myShouldDebugDraw = !myShouldDebugDraw;
    }
    if(myShouldDebugDraw)
    {
        ImGui::SetCursorPos({20, 50});
        ImGui::Text("# | Display\t| Back \t| Next");
        for(int lane = 0; lane < 7; lane++)
        {
            ImGui::Text("%i | %i\t\t| %i\t\t| %i", lane + 1, myLanes[lane].myLine, myBackLanes[lane].myLine, myAssemblyLanes[lane].myLine);
        }
        if(myBackgroundQueue.size() != 0)
        {
            ImGui::Text("");
            float start = myBackgroundQueue.front().myStartTime;
            float end = myBackgroundQueue.front().myEndTime;
            ImGui::Text("Next fade: s: %i | e: %i | a: %f", myBackgroundQueue.front().myStartTime, myBackgroundQueue.front().myEndTime, remap(clamp(playbackProgress, start, end), start, end, 0.f, 1.f));
        }
    }

    Gui_End();
}

void PreviewWindow::SetFont(ImFont *aFont)
{
    ourFont = aFont;
}

void PreviewWindow::SetRulerFont(ImFont *aFont)
{
    ourRulerFont = aFont;
}

bool PreviewWindow::GetHasVideo()
{
    return ourHasVideo;
}

void PreviewWindow::AddBackgroundElement(std::string aBGPath)
{
    DBGprintf("Loading %s.\n", aBGPath.c_str());
    if(!std::filesystem::exists(aBGPath))
    {
        printf("%s does not exist!\n", aBGPath.c_str());
        return;
    }
    if(std::filesystem::is_directory(aBGPath))
    {
        for (auto &path : std::filesystem::directory_iterator(aBGPath))
        {
            if (path.path().extension() == ".mp4" || path.path().extension() == ".png" | path.path().extension() == ".jpg")
            {
                AddBackgroundElement(path.path().string());
            }
        }
        return;
    }
    if(!aBGPath.contains("local"))
    {
        std::filesystem::copy(aBGPath, "/local", std::filesystem::copy_options::overwrite_existing);
    }
    ourHasVideo = std::filesystem::path(aBGPath).extension() == ".mp4";
    aBGPath = std::filesystem::path(aBGPath).filename().string();
    ourBackgroundPaths.push_back(aBGPath);
    ourBackgrounds[aBGPath] = {0};
    //SaveBackgroundElementsToLocal();
}

ImExtTexture PreviewWindow::GetBackgroundTexture(std::string aBGPath, bool aShouldReRender)
{
    if(!ourBackgrounds.contains(aBGPath) || ourBackgrounds[aBGPath].myID == 0)
    {
        std::string extension = std::filesystem::path(aBGPath).extension().string();
        if(extension == ".mp4")
            ImGui::Ext::LoadVideo(("##" + aBGPath).data(), ("/local/" + aBGPath).data());
        if(extension == ".png" || extension == ".jpg")
            ImGui::Ext::LoadImage(("##" + aBGPath).data(), ("/local/" + aBGPath).data());
        ourBackgrounds[aBGPath] = {};
        ImGui::Ext::RenderTexture(("##" + aBGPath).data(), ourBackgrounds[aBGPath]);
    }
    else if(aShouldReRender)
    {
        ImGui::Ext::RenderTexture(("##" + aBGPath).data(), ourBackgrounds[aBGPath]);
    }
    return ourBackgrounds[aBGPath];
}

const std::vector<std::string>& PreviewWindow::GetBackgroundElementPaths()
{
    return ourBackgroundPaths;
}

void PreviewWindow::ClearBackgroundElements()
{
    for(std::string& path : ourBackgroundPaths)
    {
        std::error_code ferr;
        std::filesystem::remove("/local/" + std::filesystem::path(path).filename().string(), ferr);
        ImGui::Ext::DeleteTexture(("##" + path).data(), ourBackgrounds[path]);
    }
    //FileHandler::SyncLocalFS();
    ourBackgrounds.clear();
    ourBackgroundPaths.clear();
}

void PreviewWindow::SetTokenFlash(bool aShouldFlash)
{
    ourTokenFlash = aShouldFlash;
}

void PreviewWindow::QueueImageFade()
{
    if(ourHasVideo) {return;}
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    std::string timeStr = StringTools::Split(doc.GetToken(myNextAddLineIndex, 0).myValue, " ")[1];
    std::string imgPath = doc.GetToken(myNextAddLineIndex, 0).myValue.substr(("image " + timeStr + " ").size());
    uint startTime = doc.GetTimedTokenAfter(myNextAddLineIndex, 0).myStartTime;
    myBackgroundQueue.push_back({imgPath, startTime, startTime + (uint)(std::stof(timeStr) * 100)});
    //myBackgroundQueue.push_back({"", 0, 0});
}

int PreviewWindow::AssembleLanes(float aWidth)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
	if(doc.GetData().size() <= myNextAddLineIndex) {return -1;}
	if(doc.GetLine(myNextAddLineIndex).size() == 0) {return 0;}
    if(doc.GetToken(myNextAddLineIndex, 0).myValue.starts_with("image ")) {QueueImageFade(); return 0;}
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
    uint lineStart = UINT_MAX;
    uint lineEnd = 0;
    for(int token = 0; token < doc.GetLine(myNextAddLineIndex).size(); token++)
    {
        if(!doc.GetToken(myNextAddLineIndex, token).myHasStart) { continue; }
        if(lineStart > doc.GetToken(myNextAddLineIndex, token).myStartTime)
        {
            lineStart = doc.GetToken(myNextAddLineIndex, token).myStartTime;
        }
        if(lineEnd < doc.GetToken(myNextAddLineIndex, token).myStartTime)
        {
            lineEnd = doc.GetToken(myNextAddLineIndex, token).myStartTime;
        }
    }
    lineStart = lineStart == UINT_MAX ? 0 : lineStart;
    int nextStartToken = 0;
    int lastSpaceToken = -1;
    for(int lane = 0; lane < 7; lane++)
    {
        if(doc.GetLine(myNextAddLineIndex).size() <= nextStartToken)
        {
            if(myAssemblyLanes[lane - 1].myWidth < 0.5f)
            {
                lane--;
                //printf("Removed an empty lane at end of line %i.\n", myNextAddLineIndex);
            }
            //printf("Line %i (size: %i) assembled and takes %i lanes\n", myNextAddLineIndex, doc.GetLine(myNextAddLineIndex).size(), lane);
            //printf("\t%s\n", doc.SerializeLineAsText(doc.GetLine(myNextAddLineIndex)).data());
            return lane;
        }
        myAssemblyLanes[lane].myLine = myNextAddLineIndex;
        myAssemblyLanes[lane].myStartToken = nextStartToken;
        myAssemblyLanes[lane].myStartTime = lineStart;
        myAssemblyLanes[lane].myEndTime = lineEnd;
        float currentTextWidth = 0;
        ImGui::PushFont(ourRulerFont);
        ourRulerFont->Scale = DPI_UNSCALED(((float)doc.GetFontSize() / 50.f));
        do
        {
            if(doc.GetLine(myNextAddLineIndex).size() <= nextStartToken)
            {
                myAssemblyLanes[lane].myWidth = currentTextWidth;
                lastSpaceToken = -1;
                break;
            }
            if(doc.IsEffectToken(doc.GetToken(myNextAddLineIndex, nextStartToken)))
            {
                nextStartToken++;
                continue;
            }
            // Multipying by 2.5 on the below lines is to go from the Main font (40 / 2) to the preview display font of 50.
            currentTextWidth += ImGui::CalcTextSize(doc.GetToken(myNextAddLineIndex, nextStartToken).myValue.data()).x;
            if(doc.GetToken(myNextAddLineIndex, nextStartToken).myValue.ends_with(" "))
            {
                if(currentTextWidth > aWidth && lastSpaceToken != -1)
                {
                    break;
                }
                myAssemblyLanes[lane].myWidth = currentTextWidth;
                lastSpaceToken = nextStartToken;
            }
            nextStartToken++;
        } while(currentTextWidth < aWidth || lastSpaceToken == -1);
        ImGui::PopFont();
        nextStartToken = lastSpaceToken == -1 ? nextStartToken : (lastSpaceToken + 1);
        myAssemblyLanes[lane].myEndToken = nextStartToken;
        lastSpaceToken = -1;
        //printf("Lane %i has size %f\n", lane, myAssemblyLanes[lane].myWidth);
    }
    //printf("Line %i (lenght: %i) assembled and takes all 7 lanes\n", myNextAddLineIndex, doc.GetLine(myNextAddLineIndex).size());
    //printf("\t%s\n", doc.SerializeLineAsText(doc.GetLine(myNextAddLineIndex)).data());
    return 7;
}

bool PreviewWindow::FillBackLanes(int aLaneCount)
{
    float scaledWidth = 640.f - 80.f; // The width of a 360p display, which ECHO seems to emulate, minus some padding on the edges. 
    int nextLineNeeds = AssembleLanes(scaledWidth);
	if(nextLineNeeds == 0)  // 0 means the line isn't valid or there's nothing to process
    {
        myNextAddLineIndex++;
        return true;
    }
	if(nextLineNeeds == -1) // -1 means stop trying
    {
        return false;
    }
    if(nextLineNeeds > aLaneCount)
    {
        Console::LogError("Line " + std::to_string(myNextAddLineIndex) + " is too long to fit on the screen. This will stop ECHO from displaying any lines after this. Please split the line or lower the font size. ", myNextAddLineIndex);
    }
    else if(nextLineNeeds > aLaneCount * .5f)
    {
        Console::LogWarning("Line " + std::to_string(myNextAddLineIndex) + " takes up more than half the screen. This could stop other lines from displaying correctly. Please split the line or lower the font size. ", myNextAddLineIndex);
    }
    int foundPlace = FillBackLanesSetLine(aLaneCount, nextLineNeeds);
    if(foundPlace == -3) // invalid
    {
        myNextAddLineIndex++;
        return true;
    }
    if(foundPlace == -2) // set <line> not yet available
    {
        return false;
    }
    if(foundPlace == -1)
    {
        for(int i = (aLaneCount / 2) + (nextLineNeeds / 2); i >= nextLineNeeds; i--)
        {
            foundPlace = i - nextLineNeeds;
            for(int j = 0; j < nextLineNeeds; j++)
            {
                if(myBackLanes[foundPlace + j].myLine != -1 || (myLanes[foundPlace + j].myLine != -1 && myAssemblyLanes[0].myStartTime < myLanes[foundPlace + j].myEndTime))
                {
                    foundPlace = -1;
                }
            }
            if(foundPlace != -1) {break;}
        }
    }
    if(foundPlace == -1)
    {
        for(int i = (aLaneCount / 2) - (nextLineNeeds / 2); i <= aLaneCount - nextLineNeeds; i++)
        {
            foundPlace = i;
            for(int j = 0; j < nextLineNeeds; j++)
            {
                if(myBackLanes[foundPlace + j].myLine != -1 || (myLanes[foundPlace + j].myLine != -1 && myAssemblyLanes[0].myStartTime < myLanes[foundPlace + j].myEndTime))
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
            //printf("Moving %i tokens from line %i to back lane %i\n", myAssemblyLanes[i].myEndToken - myAssemblyLanes[i].myStartToken, myAssemblyLanes[i].myLine, i + foundPlace);
            myBackLanes[i + foundPlace] = myAssemblyLanes[i];
            //if(myBackLanes[i + foundPlace].myEndTime < myLanes[i + foundPlace].myEndTime)// && myLanes[i + foundPlace].myLine != -1)
            //{
            //    Console::LogWarning("Line " + std::to_string(myNextAddLineIndex) + " wasn't shown as it ended before the end of line " + std::to_string(myLanes[i + foundPlace].myLine) + " which took up the same space. This could be a bug in Resonate's algorithm and might be fine in ECHO. ", myNextAddLineIndex);
            //}
            //else if(myBackLanes[i + foundPlace].myStartTime < myLanes[i + foundPlace].myEndTime)// && myLanes[i + foundPlace].myLine != -1)
            //{
            //    Console::LogWarning("Line " + std::to_string(myNextAddLineIndex) + " wasn't shown in time as it began before the end of line " + std::to_string(myLanes[i + foundPlace].myLine) + " which took up the same space. This could be a bug in Resonate's algorithm and might be fine in ECHO. ", myNextAddLineIndex);
            //}
        }
        myNextAddLineIndex++;
        return true;
    }
    return false;
}

// return value of -1 means <line> is not set, -2 means lanes not available and -3 means invalid index
int PreviewWindow::FillBackLanesSetLine(int aLaneCount, int aNextLineNeeds)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    if(doc.GetToken(myNextAddLineIndex, 0).myValue.starts_with("<line"))
    {
        int foundPlace = -2;
        int lane = std::stoi(StringTools::Split(doc.GetToken(myNextAddLineIndex, 0).myValue, std::regex("[-\\d]+"), true)[0]);
        if(lane == 0 || lane <= -aLaneCount || lane >= aLaneCount)
        {
            return -3;
        }
        if(lane < 0)
        {
            foundPlace = (aLaneCount + lane) - (aNextLineNeeds - 1);
        }
        else if(lane > 0)
        {
            foundPlace = lane - 1;
        }
        if(foundPlace < 0 || aLaneCount < foundPlace + aNextLineNeeds)
        {
            return -3;
        }
        for(int j = 0; j < aNextLineNeeds && foundPlace != -2; j++)
        {
            if(myBackLanes[foundPlace + j].myLine != -1)
            {
                foundPlace = -2;
            }
        }
        return foundPlace;
    }
    return -1;
}

// For future reference;
// The inner for loop only runs when every display lane from `currentStartLane` to `lane` are -1
bool PreviewWindow::TryDisplayLanes()
{
    int checkingLine = -1;
    int currentStartLane = -1;
    bool displayedNewLines = false;
    for(int lane = 0; lane < 7; lane++)
    {
        if(checkingLine != myBackLanes[lane].myLine)
        {
            for(int j = currentStartLane; currentStartLane != -1 && checkingLine != -1 && j < lane; j++)
            {
                //printf("Moving line %i to display lane %i\n", myBackLanes[j].myLine, j);
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

bool PreviewWindow::CheckLaneVisible(int aLane, uint someCurrentTime, uint aDelay)
{
    if(myLanes[aLane].myLine == -1) {return false;}
    //Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    //if(doc.GetToken(myLanes[aLane].myLine, 0).myHasStart)
    //{
    //    return doc.GetToken(myLanes[aLane].myLine, 0).myStartTime <= someCurrentTime + aDelay;
    //}
    //else
    //{
    //    return doc.GetTimedTokenAfter(myLanes[aLane].myLine, 0).myStartTime <= someCurrentTime + aDelay;
    //}
    //return false;
    return myLanes[aLane].myStartTime <= someCurrentTime + aDelay;
}

bool PreviewWindow::RemoveOldLanes(uint someCurrentTime, uint aDelay)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    bool output = false;
    for(int lane = 0; lane < 7; lane++)
    {
		if(myLanes[lane].myLine == -1 || doc.IsNull(doc.GetLine(myLanes[lane].myLine))) {continue;}
        if(myLanes[lane].myEndTime + aDelay < someCurrentTime)
        {
            //printf("Line %i is removed from lane %i\n", myLanes[lane].myLine, lane);

            // For some reason the backLanes are -1‽
            if(myBackLanes[lane].myEndTime < myLanes[lane].myEndTime && myBackLanes[lane].myLine != -1 && myLanes[lane].myLine != -1)
            {
                Console::LogWarning("Line " + std::to_string(myBackLanes[lane].myLine) + " wasn't shown as it ended before the end of line " + std::to_string(myLanes[lane].myLine) + " which took up the same space. This could be a bug in Resonate's algorithm and might be fine in ECHO. ", myBackLanes[lane].myLine);
            }
            else if(myBackLanes[lane].myStartTime < myLanes[lane].myEndTime && myBackLanes[lane].myLine != -1 && myLanes[lane].myLine != -1)
            {
                Console::LogWarning("Line " + std::to_string(myBackLanes[lane].myLine) + " wasn't shown in time as it began before the end of line " + std::to_string(myLanes[lane].myLine) + " which took up the same space. This could be a bug in Resonate's algorithm and might be fine in ECHO. ", myBackLanes[lane].myLine);
            }
            myLanes[lane].myLine = -1;
            output = true;
        }
    }
    return output;
}

void PreviewWindow::Resetprogress()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    int lanesShown = doc.GetFontSize() <= 43 ? 7 : doc.GetFontSize() <= 50 ? 6 : 5;
    myNextAddLineIndex = 0;
    for(int lane = 0; lane < 7; lane++)
    {
        myLanes[lane].myLine = -1;
        myBackLanes[lane].myLine = -1;
        myAssemblyLanes[lane].myLine = -1;
    }
    myBackgroundQueue.clear();
    while (FillBackLanes(lanesShown))
    {
    }
    while(TryDisplayLanes())
    {
        while (FillBackLanes(lanesShown))
        {
        }
    }
}

void PreviewWindow::SaveBackgroundElementsToLocal()
{
    for(auto& [path, tex] : ourBackgrounds)
    {
        if(!path.contains("local"))
        {
            std::filesystem::copy(path, "/local", std::filesystem::copy_options::overwrite_existing);
        }
    }
    //FileHandler::SyncLocalFS();
}
