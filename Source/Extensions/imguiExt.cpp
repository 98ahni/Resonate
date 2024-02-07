#include "imguiExt.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <Serialization/KaraokeData.h>
#include <Defines.h>

EM_JS(void, create_button, (emscripten::val id, emscripten::val event, emscripten::val callback, int pos_x, int pos_y, int width, int height), {
    let btn = document.getElementById(id);
    if(btn != 'undefined'){
        btn = document.createElement('button');
        btn.id = id;
        document.body.insertBefore(btn, document.getElementById('canvas'));
    }
    if(typeof callback == 'string'){
        btn.addEventListener(event, Module[callback]);
    }else{
        btn.addEventListener(event, _ExecCallback(callback));
    }
    btn.style.position = 'fixed';
    btn.style.left = pos_x + 'px';
    btn.style.top = pos_y + 'px';
    btn.style.width = width + 'px';
    btn.style.height = height + 'px';
});
extern"C" EMSCRIPTEN_KEEPALIVE void ExecCallback(ImGui::Ext::HTMLEvent aCallback)
{
    aCallback();
}

void ImGui::Ext::CreateHTMLButton(const char *anID, const char *anEvent, const char *aJSFunctonName)
{
    create_button(emscripten::val(anID), emscripten::val(anEvent), emscripten::val(aJSFunctonName), (int)ImGui::GetCursorPosX(), (int)ImGui::GetCursorPosY(), (int)ImGui::GetItemRectSize().x, (int)ImGui::GetItemRectSize().y);
}

void ImGui::Ext::CreateHTMLButton(const char *anID, const char *anEvent, HTMLEvent aCallback)
{
    create_button(emscripten::val(anID), emscripten::val(anEvent), emscripten::val(aCallback), (int)ImGui::GetCursorPosX(), (int)ImGui::GetCursorPosY(), (int)ImGui::GetItemRectSize().x, (int)ImGui::GetItemRectSize().y);
}

void ImGui::Ext::CreateHTMLInput(const char *anID, const char *aType, const char *anEvent, const char *aJSFunctonName)
{
}

void ImGui::Ext::CreateHTMLInput(const char *anID, const char *aType, const char *anEvent, HTMLEvent aCallback)
{
}

void ImGui::Ext::DestroyHTMLElement(const char *anID)
{
}

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
