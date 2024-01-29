#include "imguiExt.h"
#include <imgui.h>
#include <imgui_internal.h>
#include "Defines.h"

void ImGui::Ext::TimedSyllable(const char *syllable, float start, float end, float currentTime)
{
    ImVec2 size = CalcTextSize(syllable);
    ImVec2 pos = GetCursorScreenPos();
    ImVec2 timeStartPos = {pos.x, pos.y + (size.y * 1.2f)};
    ImVec2 timeEndPos = {remap(clamp(currentTime, start, end), start, end, pos.x, pos.x + size.x), pos.y + (size.y * 1.2f)};
    Text(syllable);
    ImDrawList* drawList = GetWindowDrawList();
    drawList->AddLine(timeStartPos, timeEndPos, IM_COL32_WHITE, 2);
    drawList->AddTriangleFilled(timeStartPos, {timeStartPos.x + 5, timeStartPos.y}, {timeStartPos.x, timeStartPos.y + 5}, IM_COL32_WHITE);
}

void ImGui::Ext::SetColor(unsigned int col)
{
    PushStyleColor(ImGuiCol_Text, col);
}

void ImGui::Ext::ClearColor()
{
    PopStyleColor(GetCurrentContext()->ColorStack.size());
}
