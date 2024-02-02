#include "imguiExt.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <Serialization/KaraokeData.h>
#include <Defines.h>

bool ImGui::Ext::TimedSyllable(Serialization::KaraokeToken aSyllable, float aCurrentTime)
{
    ImVec2 size = CalcTextSize(aSyllable.myValue.data());
    ImVec2 pos = GetCursorScreenPos();
    float start = aSyllable.myStartTime;
    float end = aSyllable.myEndTime;
    ImVec2 timeStartPos = {pos.x, pos.y + (size.y * 1.2f)};
    ImVec2 timeEndPos = {remap(clamp(aCurrentTime, start, end), start, end, pos.x, pos.x + size.x), pos.y + (size.y * 1.2f)};
    Text(aSyllable.myValue.data());
    bool clicked = IsItemClicked(0);
    ImDrawList* drawList = GetWindowDrawList();
    drawList->AddLine(timeStartPos, timeEndPos, IM_COL32_WHITE, 2);
    drawList->AddTriangleFilled(timeStartPos, {timeStartPos.x + 5, timeStartPos.y}, {timeStartPos.x, timeStartPos.y + 5}, IM_COL32_WHITE);
    return clicked;
}

void ImGui::Ext::SetColor(unsigned int aCol)
{
    PushStyleColor(ImGuiCol_Text, aCol);
}

void ImGui::Ext::ClearColor()
{
    PopStyleColor(GetCurrentContext()->ColorStack.size());
}
