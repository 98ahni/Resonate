//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "TouchControl.h"
#include "AudioPlayback.h"
#include "TimingEditor.h"
#include <Extensions/imguiExt.h>
#include <Defines.h>

void TouchControl::OnImGuiDraw()
{
    Gui_Begin();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetFrameHeight() * .5f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, {3, 1});
    if(ImGui::Ext::ToggleSwitch("Edit Mode", &myIsCharMode)){}
    ImGui::PopStyleVar(2);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec2 contentSize = ImGui::GetWindowSize();
    ImVec2 offset = ImVec2(0, ImGui::GetCursorPosY());
    if(ImGui::IsWindowDocked() && contentSize.x > contentSize.y - offset.y)
    {
        offset.x = contentSize.x - (contentSize.y - offset.y);
        offset.x *= .5f;
        contentSize.x = contentSize.y - offset.y;
    }
    else if(contentSize.x != myLastXSize)
    {
        ImGui::SetWindowSize({contentSize.x, contentSize.x + offset.y});
        myLastXSize = contentSize.x;
    }
    else if(contentSize.y - offset.y != myLastXSize)
    {
        ImGui::SetWindowSize({contentSize.y - offset.y, contentSize.y});
        myLastXSize = contentSize.y - offset.y;
    }
    contentSize.y -= offset.y;
    ImVec2 buttonSize = ImVec2(contentSize.x * .28f, contentSize.x * .28f);
    ImVec2 backPosMult = ImVec2(.06f, .06f);
    ImVec2 skipPosMult = ImVec2(.66f, .06f);
    ImVec2 upPosMult = ImVec2(.36f, .06f);
    ImVec2 downPosMult = ImVec2(.36f, .66f);
    ImVec2 leftPosMult = ImVec2(.06f, .36f);
    ImVec2 rightPosMult = ImVec2(.66f, .36f);
    ImVec2 togglePosMult = ImVec2(.36f, .36f);
    ImVec2 timePosMult = ImVec2(.06f, .66f);
    ImVec2 endPosMult = ImVec2(.66f, .66f);

    ImGui::SetCursorPos(contentSize * backPosMult + offset);
    if(ImGui::Button("##5sBack", buttonSize))
    {
        AudioPlayback::SetPlaybackProgress(AudioPlayback::GetPlaybackProgress() - (500.f * (AudioPlayback::GetPlaybackSpeed() * .1f)));
    }
    // 5s
    ImVec2 btnCenter = contentSize * backPosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos();
    btnCenter.x += buttonSize.x * .01f;
    btnCenter.y += buttonSize.x * .005f;
    drawList->PathArcTo({btnCenter.x - buttonSize.x * .07f, btnCenter.y + buttonSize.x * .04f}, buttonSize.x * .055f, -2.f, 2.4f);
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .03f);
    drawList->PathLineTo({btnCenter.x - buttonSize.x * .1f, btnCenter.y + buttonSize.x * .004f});
    drawList->PathLineTo({btnCenter.x - buttonSize.x * .1f, btnCenter.y - buttonSize.x * .1f});
    drawList->PathLineTo({btnCenter.x - buttonSize.x * .01f, btnCenter.y - buttonSize.x * .1f});
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .03f);
    drawList->PathArcTo({btnCenter.x + buttonSize.x * .062f, btnCenter.y - buttonSize.x * .015f}, buttonSize.x * .035f, 1.57f, 6.1f);
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .03f);
    drawList->PathArcTo({btnCenter.x + buttonSize.x * .06f, btnCenter.y + buttonSize.x * .06f}, buttonSize.x * .04f, -1.57f, 3.f);
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .03f);
    // ~5s
    // Back Arrow
    drawList->PathArcTo(contentSize * backPosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos(), buttonSize.x * .3f, -1.57f, 3.54f);
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .05f);
    ImVec2 triangleTip = ImVec2(contentSize * backPosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos());
    triangleTip.y -= buttonSize.x * .3f;
    triangleTip.x -= buttonSize.x * .1f;
    drawList->AddTriangleFilled(triangleTip, ImVec2(triangleTip.x + buttonSize.x * .12f, triangleTip.y - buttonSize.x * .1f), ImVec2(triangleTip.x + buttonSize.x * .12f, triangleTip.y + buttonSize.x * .1f), IM_COL32_WHITE);
    // ~Back Arrow
    ImGui::SetCursorPos(contentSize * skipPosMult + offset);
    if(ImGui::Button("##5sSkip", buttonSize))
    {
        AudioPlayback::SetPlaybackProgress(AudioPlayback::GetPlaybackProgress() + (500.f * (AudioPlayback::GetPlaybackSpeed() * .1f)));
    }
    // 5s
    btnCenter = contentSize * skipPosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos();
    btnCenter.x += buttonSize.x * .01f;
    btnCenter.y += buttonSize.x * .01f; // Different heights looks more centered I guess...
    drawList->PathArcTo({btnCenter.x - buttonSize.x * .07f, btnCenter.y + buttonSize.x * .04f}, buttonSize.x * .055f, -2.f, 2.4f);
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .03f);
    drawList->PathLineTo({btnCenter.x - buttonSize.x * .1f, btnCenter.y + buttonSize.x * .004f});
    drawList->PathLineTo({btnCenter.x - buttonSize.x * .1f, btnCenter.y - buttonSize.x * .1f});
    drawList->PathLineTo({btnCenter.x - buttonSize.x * .01f, btnCenter.y - buttonSize.x * .1f});
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .03f);
    drawList->PathArcTo({btnCenter.x + buttonSize.x * .062f, btnCenter.y - buttonSize.x * .015f}, buttonSize.x * .035f, 1.57f, 6.1f);
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .03f);
    drawList->PathArcTo({btnCenter.x + buttonSize.x * .06f, btnCenter.y + buttonSize.x * .06f}, buttonSize.x * .04f, -1.57f, 3.f);
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .03f);
    // ~5s
    // Skip Arrow
    drawList->PathArcTo(contentSize * skipPosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos(), buttonSize.x * .3f, -.4f, 4.71f);
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .05f);
    triangleTip = ImVec2(contentSize * skipPosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos());
    triangleTip.y -= buttonSize.x * .3f;
    triangleTip.x += buttonSize.x * .1f;
    drawList->AddTriangleFilled(triangleTip, ImVec2(triangleTip.x - buttonSize.x * .12f, triangleTip.y - buttonSize.x * .1f), ImVec2(triangleTip.x - buttonSize.x * .12f, triangleTip.y + buttonSize.x * .1f), IM_COL32_WHITE);
    // ~Skip Arrow
    ImGui::SetCursorPos(contentSize * upPosMult + offset);
    if(ImGui::Button("##Up", buttonSize))
    {
        TimingEditor::Get().MoveMarkerUp();
    }
    // Up Arrow
    triangleTip = ImVec2(contentSize * upPosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos());
    triangleTip.y -= buttonSize.x * .3f;
    drawList->AddTriangleFilled(triangleTip, ImVec2(triangleTip.x + buttonSize.x * .3f, triangleTip.y + buttonSize.x * .6f), ImVec2(triangleTip.x - buttonSize.x * .3f, triangleTip.y + buttonSize.x * .6f), IM_COL32_WHITE);
    // ~Up Arrow
    ImGui::SetCursorPos(contentSize * downPosMult + offset);
    if(ImGui::Button("##Down", buttonSize))
    {
        TimingEditor::Get().MoveMarkerDown();
    }
    // Down Arrow
    triangleTip = ImVec2(contentSize * downPosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos());
    triangleTip.y += buttonSize.x * .3f;
    drawList->AddTriangleFilled(triangleTip, ImVec2(triangleTip.x + buttonSize.x * .3f, triangleTip.y - buttonSize.x * .6f), ImVec2(triangleTip.x - buttonSize.x * .3f, triangleTip.y - buttonSize.x * .6f), IM_COL32_WHITE);
    // ~Down Arrow
    ImGui::SetCursorPos(contentSize * leftPosMult + offset);
    if(ImGui::Button("##Left", buttonSize))
    {
        TimingEditor::Get().MoveMarkerLeft(myIsCharMode);
    }
    // Left Arrow
    triangleTip = ImVec2(contentSize * leftPosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos());
    triangleTip.x -= buttonSize.x * .3f;
    drawList->AddTriangleFilled(triangleTip, ImVec2(triangleTip.x + buttonSize.x * .6f, triangleTip.y - buttonSize.x * .3f), ImVec2(triangleTip.x + buttonSize.x * .6f, triangleTip.y + buttonSize.x * .3f), IM_COL32_WHITE);
    // ~Left Arrow
    ImGui::SetCursorPos(contentSize * rightPosMult + offset);
    if(ImGui::Button("##Right", buttonSize))
    {
        TimingEditor::Get().MoveMarkerRight(myIsCharMode);
    }
    // Right Arrow
    triangleTip = ImVec2(contentSize * rightPosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos());
    triangleTip.x += buttonSize.x * .3f;
    drawList->AddTriangleFilled(triangleTip, ImVec2(triangleTip.x - buttonSize.x * .6f, triangleTip.y - buttonSize.x * .3f), ImVec2(triangleTip.x - buttonSize.x * .6f, triangleTip.y + buttonSize.x * .3f), IM_COL32_WHITE);
    // ~Right Arrow
    ImGui::SetCursorPos(contentSize * togglePosMult + offset);
    if(ImGui::Button("##Add/Remove", buttonSize))
    {
        TimingEditor::Get().ToggleTokenHasTime();
    }
    // Toggle Token Icon
    ImVec2 linePos = ImVec2(contentSize * togglePosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos());
    drawList->PathLineTo({linePos.x - buttonSize.x * .15f, linePos.y - buttonSize.x * .3f});
    drawList->PathLineTo({linePos.x - buttonSize.x * .3f, linePos.y - buttonSize.x * .3f});
    drawList->PathLineTo({linePos.x - buttonSize.x * .3f, linePos.y + buttonSize.x * .3f});
    drawList->PathLineTo({linePos.x - buttonSize.x * .15f, linePos.y + buttonSize.x * .3f});
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .05f);
    drawList->PathLineTo({linePos.x + buttonSize.x * .15f, linePos.y - buttonSize.x * .3f});
    drawList->PathLineTo({linePos.x + buttonSize.x * .3f, linePos.y - buttonSize.x * .3f});
    drawList->PathLineTo({linePos.x + buttonSize.x * .3f, linePos.y + buttonSize.x * .3f});
    drawList->PathLineTo({linePos.x + buttonSize.x * .15f, linePos.y + buttonSize.x * .3f});
    drawList->PathStroke(IM_COL32_WHITE, 0, buttonSize.x * .05f);
    drawList->AddCircleFilled({linePos.x, linePos.y - buttonSize.x * .15f}, buttonSize.x * .07f, IM_COL32_WHITE);
    drawList->AddCircleFilled({linePos.x, linePos.y + buttonSize.x * .2f}, buttonSize.x * .07f, IM_COL32_WHITE);
    // ~Togle Token Icon
    ImGui::SetCursorPos(contentSize * timePosMult + offset);
    if(ImGui::Button("##Time", buttonSize))
    {
        TimingEditor::Get().RecordStartTime();
    }
    // Time Arrow
    triangleTip = ImVec2(contentSize * timePosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos());
    triangleTip.x -= buttonSize.x * .3f;
    triangleTip.y -= buttonSize.x * .3f;
    drawList->AddTriangleFilled(triangleTip, ImVec2(triangleTip.x, triangleTip.y + buttonSize.x * .6f), ImVec2(triangleTip.x + buttonSize.x * .6f, triangleTip.y + buttonSize.x * .6f), IM_COL32_WHITE);
    // ~Time Arrow
    ImGui::SetCursorPos(contentSize * endPosMult + offset);
    if(ImGui::Button("##End", buttonSize))
    {
        TimingEditor::Get().RecordEndTime();
    }
    // End Arrow
    triangleTip = ImVec2(contentSize * endPosMult + offset + (buttonSize * .5f) + ImGui::GetWindowPos());
    triangleTip.x += buttonSize.x * .3f;
    triangleTip.y -= buttonSize.x * .3f;
    drawList->AddTriangleFilled(triangleTip, ImVec2(triangleTip.x - buttonSize.x * .6f, triangleTip.y + buttonSize.x * .6f), ImVec2(triangleTip.x, triangleTip.y + buttonSize.x * .6f), IM_COL32_WHITE);
    // ~End Arrow
    Gui_End();
}