#include "imguiExt.h"
#include <imgui.h>
#include <imgui_internal.h>

#define remap(value, istart, istop, ostart, ostop) (ostart + (ostop - ostart) * ((value - istart) / (istop - istart)))
#define clamp(value, min, max) (value < min ? min : (max < value ? max : value))

void ImGui::Ext::TimedSyllable(const char *syllable, float start, float end, float currentTime)
{
    ImVec2 size = CalcTextSize(syllable);
    ImVec2 pos = GetCursorScreenPos();
    ImVec2 timeStartPos = {pos.x, pos.y + (size.y * 1.2f)};
    ImVec2 timeEndPos = {remap(clamp(currentTime, start, end), start, end, pos.x, pos.x + size.x), pos.y + (size.y * 1.2f)};
    Text(syllable);
    ImDrawList* drawList = GetWindowDrawList();
    drawList->AddLine(timeStartPos, timeEndPos, IM_COL32_WHITE, 2);
}

void ImGui::Ext::SetColor(unsigned int col)
{
}

void ImGui::Ext::ClearColor()
{
}
