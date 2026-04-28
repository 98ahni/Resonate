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
            while(RemoveOldLanes(INT_MAX))
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
        if(std::filesystem::path(doc.GetName()).replace_extension("") == fpath.filename().replace_extension(""))
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
    ourTokenFlash = Serialization::Preferences::HasKey("Preview/TokenFlash") && Serialization::Preferences::GetBool("Preview/TokenFlash");
    ourUseOutline = !Serialization::Preferences::HasKey("Preview/UseOutline") || Serialization::Preferences::GetBool("Preview/UseOutline");
}

void PreviewWindow::OnImGuiDraw()
{
    ImGui::SetNextWindowSize({std::min(MainWindow::SwapWidth * .85f, DPI_SCALED(800.f)), std::min(MainWindow::SwapHeight * .8f, DPI_SCALED(450.f))}, ImGuiCond_Once);
    Gui_Begin(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if(ourHasVideo && ImGui::Ext::IsVideoPaused("##PreviewBackground") && AudioPlayback::GetIsPlaying())
    {
        EM_ASM(audio_element_pause(););
    }
    else if(ourHasVideo && !ImGui::Ext::IsVideoPaused("##PreviewBackground") && !AudioPlayback::GetIsPlaying())
    {
        EM_ASM(audio_element_play(););
    }
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 contentOffset = ImGui::GetCursorPos();
    ImVec2 contentSize = {windowSize.x - contentOffset.x, windowSize.y - contentOffset.y};
    ImVec2 aspect = {0.5625f, 1.777777f};
    if(contentSize.x < contentSize.y * aspect.y)
    {
        contentSize.y = contentSize.x * aspect.x;
        //ImGui::SetCursorPosY((windowSize.y - contentSize.y) * .5f);
        ImGui::SetCursorPos({(windowSize.x - contentSize.x) * .5f, ((windowSize.y - contentSize.y) * .5f) + ImGui::GetStyle().WindowPadding.y});
        contentOffset = ImGui::GetCursorPos();
    }
    else
    {
        contentSize.x = contentSize.y * aspect.y;
        //ImGui::SetCursorPosX((windowSize.x - contentSize.x) * .5f);
        ImGui::SetCursorPos({(windowSize.x - contentSize.x) * .5f, ((windowSize.y - contentSize.y) * .5f) + ImGui::GetStyle().WindowPadding.y});
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
    while(myBackgroundQueue.size() != 0 && playbackProgress > myBackgroundQueue.front().myEndTime)
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
        ImGui::ImageWithBg(GetBackgroundTexture(myBackgroundQueue.front().myImagePath).myID, contentSize, {0, 0}, {1, 1}, {0, 0, 0, 0}, {1, 1, 1, alpha});
    }

	//float lanePosY = ImGui::GetTextLineHeightWithSpacing();
	float lanePosY = contentSize.y / (float)lanesShown;
    float laneHeight = (lanePosY - ImGui::GetTextLineHeightWithSpacing()) * .5f;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, DPI_SCALED(10)});
    bool hasNoEffect = false;
    bool isDirect = doc.GetUseDirectText();
    for(int lane = 0; lane < 7; lane++)
    {
        int showTime = myLanes[lane].myStartTime + myLanes[lane].myShowTimeOffset;
        if(!CheckLaneVisible(lane, playbackProgress)) {continue;}
        ImGui::SetCursorPosY((lanePosY * lane) + laneHeight + contentOffset.y);
        float cursorStartX = ((contentSize.x - (myLanes[lane].myWidth * DPI_SCALED(textScale))) * .5f) + contentOffset.x;
        ImGui::SetCursorPosX(cursorStartX);

        for(int token = myLanes[lane].myStartToken; token < myLanes[lane].myEndToken; token++)
        {
            uint start = doc.GetToken(myLanes[lane].myLine, token).myStartTime;
            uint end = doc.GetTimedTokenAfter(myLanes[lane].myLine, token).myStartTime;
            if(!doc.ParseEffectToken(doc.GetToken(myLanes[lane].myLine, token)) && playbackProgress >= showTime)
            {
                showTime += (isDirect ? 0 : ourPerCharAnimTime) * doc.GetToken(myLanes[lane].myLine, token).myValue.size();
                ImGui::Ext::TimedSyllable(doc.GetToken(myLanes[lane].myLine, token).myValue, start, end, playbackProgress, false, ourTokenFlash, true, ourUseOutline ? DPI_SCALED(2 * textScale) : 0, hasNoEffect ? 1 : 1.15f);
                ImGui::SameLine();
            }
            else if(doc.GetToken(myLanes[lane].myLine, token).myValue.starts_with("<no effect>"))
            {
                isDirect = true;
                hasNoEffect = true;
            }
            else if(doc.GetToken(myLanes[lane].myLine, token).myValue.starts_with("<direct>"))
            {
                isDirect = true;
            }
            else if(doc.GetToken(myLanes[lane].myLine, token).myValue.starts_with("<cascade>"))
            {
                isDirect = false;
            }
        }
        if(lane >= lanesShown) {break;}
        ImGui::NewLine();
        if(lane == 7 || myLanes[lane].myLine != myLanes[lane + 1].myLine)
        {
            doc.PopColor();
            hasNoEffect = false;
            isDirect = doc.GetUseDirectText();
        }
    }
    ImGui::PopStyleVar();
    while(RemoveOldLanes(playbackProgress))
    {
        while(TryDisplayLanes())
        {
        }
        while (FillBackLanes(lanesShown))
        {
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
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->ChannelsSplit(2);
        drawList->ChannelsSetCurrent(1);
        if(ImGui::IsKeyDown(ImGuiKey_ModShift))
        {
            ImGui::Text("# | Display(e)\t| Back(s|e)\t\t| Next(s)");
        }
        else
        {
            ImGui::Text("# | Display\t\t| Back \t\t| Next");
        }
        float maxWidth = ImGui::GetItemRectSize().x;
        for(int lane = 0; lane < 7; lane++)
        {
            if(ImGui::IsKeyDown(ImGuiKey_ModShift))
            {
                        //  lane dis end   bl strt end  al strt     lane            dis                     end                     bl                          strt                            end                         al                              strt
                ImGui::Text("%i | %i(%i)\t| %i(%i|%i)\t| %i(%i)", lane + 1, myLanes[lane].myLine, myLanes[lane].myEndTime, myBackLanes[lane].myLine, myBackLanes[lane].myStartTime, myBackLanes[lane].myEndTime, myAssemblyLanes[lane].myLine, myAssemblyLanes[lane].myStartTime);
            }
            else
            {
                ImGui::Text("%i | %i\t| %i\t| %i", lane + 1, myLanes[lane].myLine, myBackLanes[lane].myLine, myAssemblyLanes[lane].myLine);
            }
            maxWidth = std::max(maxWidth, ImGui::GetItemRectSize().x);
        }
        if(myBackgroundQueue.size() != 0)
        {
            ImGui::Text("");
            float start = myBackgroundQueue.front().myStartTime;
            float end = myBackgroundQueue.front().myEndTime;
            ImGui::Text("Next fade: s: %i | e: %i | a: %f", myBackgroundQueue.front().myStartTime, myBackgroundQueue.front().myEndTime, remap(clamp(playbackProgress, start, end), start, end, 0.f, 1.f));
            maxWidth = std::max(maxWidth, ImGui::GetItemRectSize().x);
        }
        if(myRecalculateQueue.size() != 0)
        {
            ImGui::Text("");
            ImGui::Text("Left to recalculate:");
            for(int i = 0; i < myRecalculateQueue.size(); i++)
            {
                if(ImGui::IsKeyDown(ImGuiKey_ModShift))
                {
                    ImGui::Text("\t%i (%i)", myRecalculateQueue[i].myLine, myRecalculateQueue[i].myStartTime);
                }
                else
                {
                    ImGui::Text("\t%i", myRecalculateQueue[i].myLine);
                }
            }
        }
        if(ImGui::IsKeyDown(ImGuiKey_ModShift))
        {
            ImGui::Text("Time variance: s: %i | e: %i", ourLineAnimInTime, ourLineAnimOutTime);
        }
        else
        {
            ImGui::Text("Hold Shift for times");
        }
        drawList->ChannelsSetCurrent(0);
        ImVec2 wPos = ImGui::GetWindowPos();
        drawList->AddRectFilled({wPos.x + 10, wPos.y + 45}, {wPos.x + maxWidth + 25, wPos.y + ImGui::GetCursorPosY() + 5}, IM_COL32(0, 0, 0, 127));
        drawList->ChannelsMerge();
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
    int showTimeOffset = -ourLineAnimInTime;
    bool isDirect = doc.GetUseDirectText();
    for(int lane = 0; lane < 7; lane++)
    {
        if(doc.GetLine(myNextAddLineIndex).size() <= nextStartToken)
        {
            if(myAssemblyLanes[lane - 1].myWidth < 0.5f)
            {
                lane--;
            }
            return lane;
        }
        myAssemblyLanes[lane].myLine = myNextAddLineIndex;
        myAssemblyLanes[lane].myStartToken = nextStartToken;
        myAssemblyLanes[lane].myStartTime = lineStart;
        myAssemblyLanes[lane].myEndTime = lineEnd;
        myAssemblyLanes[lane].myShowTimeOffset = isDirect ? -ourLineAnimInTime : showTimeOffset;
        float currentTextWidth = 0;
        int currentCharCount = 0;
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
            std::string tokenValue = doc.GetToken(myNextAddLineIndex, nextStartToken).myValue;
            if(doc.IsEffectToken(doc.GetToken(myNextAddLineIndex, nextStartToken)))
            {
                if(tokenValue.starts_with("<no effect>") || tokenValue.starts_with("<direct>"))
                {
                    isDirect = true;
                }
                if(tokenValue.starts_with("<cascade>"))
                {
                    isDirect = false;
                }
                nextStartToken++;
                continue;
            }
            // Multipying by 2.5 on the below lines is to go from the Main font (40 / 2) to the preview display font of 50.
            currentTextWidth += ImGui::CalcTextSize(tokenValue.data()).x;
            currentCharCount += tokenValue.size();
            if(tokenValue.ends_with(" "))
            {
                if(currentTextWidth > aWidth && lastSpaceToken != -1)
                {
                    break;
                }
                myAssemblyLanes[lane].myWidth = currentTextWidth;
                lastSpaceToken = nextStartToken;
                showTimeOffset += currentCharCount * ourPerCharAnimTime;
                currentCharCount = 0;
            }
            nextStartToken++;
        } while(currentTextWidth < aWidth || lastSpaceToken == -1);
        ImGui::PopFont();
        nextStartToken = lastSpaceToken == -1 ? nextStartToken : (lastSpaceToken + 1);
        myAssemblyLanes[lane].myEndToken = nextStartToken;
        lastSpaceToken = -1;
        if(doc.GetThisOrPreviousTimedToken(myNextAddLineIndex, nextStartToken).myStartTime < lineStart + showTimeOffset)
        {
            Console::LogError("Line " + std::to_string(myNextAddLineIndex) + " finished before it could be animated in. Consider using <direct> or turning on Direct Text in Effects > Properties.", myNextAddLineIndex);
        }
    }
    return 7;
}

// return value of -1 means <line> is not set, -2 means lanes not available and -3 means invalid index
int PreviewWindow::FindOpenBackLanes(int aLaneCount, int aNextLineNeeds, uint aLineStartTime)
{
    uint lowestCost = UINT_MAX;
    int bestFoundPlace = -1;
    for(int i = (aLaneCount / 2) + (aNextLineNeeds / 2); i >= aNextLineNeeds; i--)
    {
        uint currentHighestCost = 0;
        bool hasBackLane = false;
        int foundPlace = i - aNextLineNeeds;
        for(int j = 0; j < aNextLineNeeds; j++)
        {
            if(myBackLanes[foundPlace + j].myLine != -1)
            {
                hasBackLane = true;
                // Check back lanes too as it might be best to wait
                if((myBackLanes[foundPlace + j].myEndTime + ourLineAnimInTime + ourLineAnimOutTime) < aLineStartTime)
                {
                    // Line doesn't have a cost as it'll be free when this line renders
                    continue;
                }
                // The line can't display here without delay so record the delay it needs
                currentHighestCost = std::max(currentHighestCost, (myBackLanes[foundPlace + j].myEndTime + ourLineAnimInTime + ourLineAnimOutTime) - aLineStartTime);
            }
            else
            {
                if(myLanes[foundPlace + j].myLine == -1 || (myLanes[foundPlace + j].myEndTime + ourLineAnimInTime + ourLineAnimOutTime) < aLineStartTime)
                {
                    // Line doesn't have a cost as it'll be free when this line renders
                    continue;
                }
                // The line can't display here without delay so record the delay it needs
                currentHighestCost = std::max(currentHighestCost, (myLanes[foundPlace + j].myEndTime + ourLineAnimInTime + ourLineAnimOutTime) - aLineStartTime);
            }
        }
        if(currentHighestCost == 0)
        {
            return hasBackLane ? -2 : foundPlace;
        }
        if(currentHighestCost <= lowestCost)
        {
            lowestCost = currentHighestCost;
            bestFoundPlace = hasBackLane ? -2 : foundPlace;
        }
    }
    for(int i = (aLaneCount / 2) - (aNextLineNeeds / 2); i <= aLaneCount - aNextLineNeeds; i++)
    {
        uint currentHighestCost = 0;
        bool hasBackLane = false;
        int foundPlace = i;
        for(int j = 0; j < aNextLineNeeds; j++)
        {
            if(myBackLanes[foundPlace + j].myLine != -1)
            {
                hasBackLane = true;
                // Check back lanes too as it might be best to wait
                if((myBackLanes[foundPlace + j].myEndTime + ourLineAnimInTime + ourLineAnimOutTime) < aLineStartTime)
                {
                    // Line doesn't have a cost as it'll be free when this line renders
                    continue;
                }
                // The line can't display here without delay so record the delay it needs
                currentHighestCost = std::max(currentHighestCost, (myBackLanes[foundPlace + j].myEndTime + ourLineAnimInTime + ourLineAnimOutTime) - aLineStartTime);
            }
            else
            {
                if(myLanes[foundPlace + j].myLine == -1 || (myLanes[foundPlace + j].myEndTime + ourLineAnimInTime + ourLineAnimOutTime) < aLineStartTime)
                {
                    // Line doesn't have a cost as it'll be free when this line renders
                    continue;
                }
                // The line can't display here without delay so record the delay it needs
                currentHighestCost = std::max(currentHighestCost, (myLanes[foundPlace + j].myEndTime + ourLineAnimInTime + ourLineAnimOutTime) - aLineStartTime);
            }
        }
        if(currentHighestCost == 0)
        {
            return hasBackLane ? -2 : foundPlace;
        }
        if(currentHighestCost <= lowestCost)
        {
            lowestCost = currentHighestCost;
            bestFoundPlace = hasBackLane ? -2 : foundPlace;
        }
    }
    return bestFoundPlace;
}

bool PreviewWindow::FillBackLanes(int aLaneCount)
{
    //float scaledWidth = 640.f - 80.f; // The width of a 360p display, which ECHO seems to emulate, minus some padding on the edges. 
    float scaledWidth = 640.f - 95.f; // The width of a 360p display, which ECHO seems to emulate, minus some padding on the edges. 
    if(RecalculateBackLanes(aLaneCount))
    {
        return false; // There are still more to be recalculated but they don't fit right now 
    }
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
        Console::LogError("Line " + std::to_string(myNextAddLineIndex) + " is too long to fit on the screen. This will stop ECHO from displaying any lines after this. Please split the line or lower the font size. \nLine needs: " + std::to_string(nextLineNeeds), myNextAddLineIndex);
    }
    else if(nextLineNeeds > aLaneCount * .5f)
    {
        Console::LogWarning("Line " + std::to_string(myNextAddLineIndex) + " takes up more than half the screen. This could stop other lines from displaying correctly. Please split the line or lower the font size. \nLine needs: " + std::to_string(nextLineNeeds), myNextAddLineIndex);
    }
    int foundPlace = FillBackLanesSetLine(aLaneCount, nextLineNeeds);
    if(foundPlace == -1)
    {
        foundPlace = FindOpenBackLanes(aLaneCount, nextLineNeeds, myAssemblyLanes[0].myStartTime);
    }
    if(foundPlace == -2) // set <line> or lowest cost not yet available
    {
        return false;
    }
    if(foundPlace == -3) // invalid
    {
        myNextAddLineIndex++;
        return true;
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
        bool needsRecalc = false;
        for(int j = 0; j < aNextLineNeeds && foundPlace != -2; j++)
        {
            if(myBackLanes[foundPlace + j].myLine != -1)
            {
                if((myBackLanes[foundPlace + j].myEndTime + ourLineAnimInTime + ourLineAnimOutTime) < myAssemblyLanes[0].myStartTime)
                {
                    foundPlace = -2;
                    continue;
                }
                if(doc.GetToken(myBackLanes[foundPlace + j].myLine, 0).myValue.starts_with("<line"))
                {
                    foundPlace = -2;
                    continue;
                }
                needsRecalc = true;
                // If lanes are taken, check if that lane ends before this starts or if it also starts with <line
                //   If so, foundPlace = -2
                //   Otherwise, move all lines that don't start with <line to a queue to be re-added
            }
        }
        if(needsRecalc && foundPlace != -2)
        {
            DBGprintf("Recalc needed\n");
            QueueBackLanesToRecalculate();
        }
        return foundPlace;
    }
    return -1;
}

void PreviewWindow::QueueBackLanesToRecalculate()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    for(int i = 0; i < 7; i++)
    {
        if(myBackLanes[i].myLine != -1 && !doc.GetToken(myBackLanes[i].myLine, 0).myValue.starts_with("<line"))
        {
            DBGprintf("Queuing lane %i (line %i) for recalc\n%s\n", i, myBackLanes[i].myLine, doc.SerializeLineAsText(doc.GetLine(myBackLanes[i].myLine)).data());
            bool couldSort = false;
            for(int j = 0; j < myRecalculateQueue.size(); j++)
            {
                if(myRecalculateQueue[j].myLine > myBackLanes[i].myLine)
                {
                    myRecalculateQueue.insert(myRecalculateQueue.begin() + j, myBackLanes[i]);
                    couldSort = true;
                    break;
                }
            }
            if(!couldSort)
            {
                myRecalculateQueue.emplace_back(myBackLanes[i]);
            }
            myBackLanes[i].myLine = -1;
        }
    }
}

bool PreviewWindow::RecalculateBackLanes(int aLaneCount)
{
    int foundPlace = 0;
    while(foundPlace >= 0)
    {
        if(!myRecalculateQueue.size())
        {
            return false;
        }
        int nextLineNeeds = 0;
        int checkingLine = -1;
        do
        {
            if(checkingLine == -1)
            {
                checkingLine = myRecalculateQueue[nextLineNeeds].myLine;
            }
            nextLineNeeds++;
        } while (myRecalculateQueue.size() > nextLineNeeds && checkingLine == myRecalculateQueue[nextLineNeeds].myLine);
        foundPlace = FindOpenBackLanes(aLaneCount, nextLineNeeds, myRecalculateQueue.front().myStartTime);
        if(foundPlace >= 0)
        {
            for(int i = 0; i < nextLineNeeds; i++)
            {
                myBackLanes[i + foundPlace] = myRecalculateQueue.front();
                myRecalculateQueue.pop_front();
            }
        }
    }
    return myRecalculateQueue.size();
}

// For future reference;
// The inner for loop only runs when every display lane from `currentStartLane` to `lane` are -1
bool PreviewWindow::TryDisplayLanes()
{
    int checkingLine = -1;
    int currentStartLane = -1;
    bool displayedNewLines = false;
    for(int lane = 0; lane <= 7; lane++)        // This is kinda scarry, it's checking lane = 7. The arrays only go from 0 through 6. Moving the = four lines down where it should be (j < lane) breaks EVERYTHING!
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

bool PreviewWindow::CheckLaneVisible(int aLane, uint someCurrentTime)
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
    return myLanes[aLane].myStartTime <= someCurrentTime + ourLineAnimInTime;
}

bool PreviewWindow::RemoveOldLanes(uint someCurrentTime)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    bool output = false;
    for(int lane = 0; lane < 7; lane++)
    {
		if(myLanes[lane].myLine == -1 || doc.IsNull(doc.GetLine(myLanes[lane].myLine))) {continue;}
        if(myLanes[lane].myEndTime + ourLineAnimOutTime < someCurrentTime)
        {
            //printf("Line %i is removed from lane %i\n", myLanes[lane].myLine, lane);

            // For some reason the backLanes are -1‽
            if(myBackLanes[lane].myLine != -1 && myLanes[lane].myLine != -1)
            {
                uint overlap = (myLanes[lane].myEndTime + ourLineAnimInTime + ourLineAnimOutTime) - myBackLanes[lane].myStartTime;
                if(myBackLanes[lane].myEndTime < myLanes[lane].myEndTime)
                {
                    Console::LogError("Line " + std::to_string(myBackLanes[lane].myLine) + " wasn't shown as it ended before the end of line " + std::to_string(myLanes[lane].myLine) + " which took up the same space.\n"
                    "This is unlikely to work in ECHO. \nOverlap: " + std::to_string(overlap / 100) + "." + std::to_string(overlap % 100) + " seconds. ", myBackLanes[lane].myLine);
                }
                else if(myBackLanes[lane].myStartTime < myLanes[lane].myEndTime)
                {
                    Console::LogWarning("Line " + std::to_string(myBackLanes[lane].myLine) + " wasn't shown in time as it began before the end of line " + std::to_string(myLanes[lane].myLine) + " which took up the same space.\n"
                    "This probably won't look good in ECHO. \nOverlap: " + std::to_string(overlap / 100) + "." + std::to_string(overlap % 100) + " seconds. ", myBackLanes[lane].myLine);
                }
                else if(myBackLanes[lane].myStartTime < (myLanes[lane].myEndTime + ourLineAnimInTime + ourLineAnimOutTime))
                {
                    Console::Log("Line " + std::to_string(myBackLanes[lane].myLine) + " wasn't shown in time as it should display before removal of line " + std::to_string(myLanes[lane].myLine) + " which took up the same space.\n"
                    "This might be fine in ECHO. \nOverlap: " + std::to_string(overlap / 100) + "." + std::to_string(overlap % 100) + " seconds. ", myBackLanes[lane].myLine);
                }
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
    myRecalculateQueue.clear();
    while (FillBackLanes(lanesShown))
    {
    }
    while(TryDisplayLanes())
    {
        while (FillBackLanes(lanesShown))
        {
        }
    }
    while(RemoveOldLanes(AudioPlayback::GetPlaybackProgress()))
    {
        while(TryDisplayLanes())
        {
            while (FillBackLanes(lanesShown))
            {
            }
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
