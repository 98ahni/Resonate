//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include "Console.h"
#include <Serialization/KaraokeData.h>
#include <Defines.h>
#include "Preview.h"
#include "AudioPlayback.h"

void Console::SearchForErrors()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    uint lastLineStartTime = 0;
    for(int line = 0; line < doc.GetData().size(); line++)
    {
        uint lineStartTime = 0;
        uint lastTokenStartTime = 0;
        bool hasSeenTimedToken = false;
        for(int token = 0; token < doc.GetLine(line).size(); token++)
        {
            Serialization::KaraokeToken& currToken = doc.GetToken(line, token);
            if(!hasSeenTimedToken && currToken.myHasStart)
            {
                lineStartTime = currToken.myStartTime;
                hasSeenTimedToken = true;
            }
            if(currToken.myHasStart)
            {
                if(currToken.myStartTime < lastTokenStartTime)
                {
                    LogWarning("Line " + std::to_string(line) + " has syllables' start times out of order. ", line);
                }
                if(!doc.IsPauseToken(currToken))
                {
                    lastTokenStartTime = currToken.myStartTime;
                }
            }
        }
        if(doc.GetLine(line).size() > 1 && !doc.IsPauseToken(doc.GetLine(line).back()))
        {
            LogError("Line " + std::to_string(line) + " does not have an end time. This will look weird in ECHO. ", line);
        }
        if(hasSeenTimedToken)
        {
            if(lineStartTime < lastLineStartTime)
            {
                LogWarning("Line " + std::to_string(line) + " has an earlier start time than the last timed line above it. It might not show up in ECHO.", line);
            }
            lastLineStartTime = lineStartTime;
        }
    }
}

void Console::Log(std::string aMessage, int aLine)
{
    for(LogContent& log : ourLogs)
    {
        if(log.myLine == aLine && log.mySeverity == Severity::Message && log.myMessage == aMessage)
        {
            return;
        }
    }
    ourLineToLogInds[aLine].push_back(ourLogs.size());
    ourLogs.push_back({Severity::Message, aMessage, aLine});
}

void Console::LogWarning(std::string aMessage, int aLine)
{
    for(LogContent& log : ourLogs)
    {
        if(log.myLine == aLine && log.mySeverity == Severity::Warning && log.myMessage == aMessage)
        {
            return;
        }
    }
    ourLineToLogInds[aLine].push_back(ourLogs.size());
    ourLogs.push_back({Severity::Warning, aMessage, aLine});
}

void Console::LogError(std::string aMessage, int aLine)
{
    for(LogContent& log : ourLogs)
    {
        if(log.myLine == aLine && log.mySeverity == Severity::Error && log.myMessage == aMessage)
        {
            return;
        }
    }
    ourLineToLogInds[aLine].push_back(ourLogs.size());
    ourLogs.push_back({Severity::Error, aMessage, aLine});
}

void Console::LineMargin(int aLine)
{
    Severity highest = (Severity)-1;
    for(int i = 0; ourLineToLogInds.contains(aLine) && i < ourLineToLogInds[aLine].size(); i++)
    {
        if(highest < ourLogs[ourLineToLogInds[aLine][i]].mySeverity)
            highest = ourLogs[ourLineToLogInds[aLine][i]].mySeverity;
    }
    DrawCompactIcon(highest, ImGui::GetTextLineHeight());
    if(highest != (Severity)-1 && ImGui::BeginItemTooltip())
    {
        ImGui::Separator();
        ImGui::PushFont(ImGui::GetIO().FontDefault);
        for(int i = 0; ourLineToLogInds.contains(aLine) && i < ourLineToLogInds[aLine].size(); i++)
        {
            int logInd = ourLineToLogInds[aLine][i];
            DrawIcon(ourLogs[logInd].mySeverity, ImGui::GetTextLineHeight());
            ImGui::SameLine();
            ImGui::Text((ourLogs[logInd].myMessage.size() > 50 ? ourLogs[logInd].myMessage.substr(0, 50) + "..." : ourLogs[logInd].myMessage).data());
            ImGui::Separator();
        }
        ImGui::PopFont();
        ImGui::EndTooltip();
    }
}

void Console::DrawIcon(Severity aType, float aSize)
{
    ImDrawList& drawList = *ImGui::GetWindowDrawList();
    ImVec2 drawPos = ImGui::GetCursorScreenPos();
    ImGui::Dummy({aSize, aSize});
    switch (aType)
    {
    case Message:
        drawList.AddRectFilled({drawPos.x, drawPos.y + (aSize * .2f)}, {drawPos.x + aSize, drawPos.y + (aSize * .8f)}, IM_COL32(150, 150, 150, 255), aSize * .1f);
        break;
    case Warning:
        drawList.AddTriangleFilled({drawPos.x + (aSize * .5f), drawPos.y + (aSize * .1f)}, {drawPos.x, drawPos.y + (aSize * .9f)}, {drawPos.x + aSize, drawPos.y + (aSize * .9f)}, IM_COL32(255, 255, 0, 255));
        break;
    case Error:
        drawList.AddCircleFilled({drawPos.x + (aSize * .5f), drawPos.y + (aSize * .5f)}, aSize * .5f, IM_COL32(255, 50, 50, 255));
        break;
    }
}

void Console::DrawCompactIcon(Severity aType, float aSize)
{
    ImDrawList& drawList = *ImGui::GetWindowDrawList();
    ImVec2 drawPos = ImGui::GetCursorScreenPos();
    ImGui::Dummy({aSize * .4f, aSize});
    switch (aType)
    {
    case Message:
        drawList.AddRectFilled(drawPos, {drawPos.x + (aSize * .2f), drawPos.y + aSize}, IM_COL32(150, 150, 150, 255), aSize * .1f);
        break;
    case Warning:
        drawList.AddRectFilled(drawPos, {drawPos.x + (aSize * .2f), drawPos.y + aSize}, IM_COL32(255, 255, 0, 255), aSize * .1f);
        break;
    case Error:
        drawList.AddRectFilled(drawPos, {drawPos.x + (aSize * .2f), drawPos.y + aSize}, IM_COL32(255, 50, 50, 255), aSize * .1f);
        break;
    }
}

ConsoleWindow::ConsoleWindow()
{
}

void ConsoleWindow::OnImGuiDraw()
{
    Gui_Begin(ImGuiWindowFlags_MenuBar);
    if(ImGui::BeginMenuBar())
    {
        if(ImGui::MenuItem("Validate"))
        {
            Console::SearchForErrors();
            PreviewWindow p(true);
            auto [peak, mean] = AudioPlayback::GetVolumeDB();
            Console::Log("The audio has a peak volume of " + std::to_string(peak) + "db and a mean of " + std::to_string(mean) + "db. ");
        }
        if(ImGui::MenuItem("Clear"))
        {
            Console::ourLineToLogInds.clear();
            Console::ourLogs.clear();
        }
        ImGui::EndMenuBar();
    }
    for(int logInd = 0; logInd < Console::ourLogs.size(); logInd++)
    {
        if(ImGui::BeginChild(("##log" + std::to_string(logInd)).data(), {0, myExpandedLog == logInd ? 0 : ImGui::GetTextLineHeight() * 2 + ImGui::GetFrameHeightWithSpacing()},
            ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            Console::DrawIcon(Console::ourLogs[logInd].mySeverity, ImGui::GetTextLineHeight() * 3);
            ImGui::SameLine();
            ImGui::TextWrapped(Console::ourLogs[logInd].myMessage.data());
        }
        ImGui::EndChild();
        if(ImGui::IsItemClicked())
        {
            myExpandedLog = myExpandedLog == logInd ? -1 : logInd;
        }
    }
    Gui_End();
}
