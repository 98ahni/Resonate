#include "imguiExt.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <Serialization/KaraokeData.h>
#include <Defines.h>

EM_JS(void, create_button, (emscripten::EM_VAL id, emscripten::EM_VAL event, emscripten::EM_VAL callback, int pos_x, int pos_y, int width, int height), {
    let btn = document.getElementById(Emval.toValue(id));
    if(btn === null){
        btn = document.createElement('button');
        btn.id = Emval.toValue(id);
        document.body.insertBefore(btn, document.getElementById('canvas').nextSibling);
    }
    btn.addEventListener(Emval.toValue(event), window[Emval.toValue(callback)], true);
    btn.style.position = 'fixed';
    btn.style.left = pos_x + 'px';
    btn.style.top = pos_y + 'px';
    btn.style.width = width + 'px';
    btn.style.height = height + 'px';
    btn.style.opacity = 0;
});
EM_JS(void, create_input, (emscripten::EM_VAL id, emscripten::EM_VAL type, emscripten::EM_VAL event, emscripten::EM_VAL callback, int pos_x, int pos_y, int width, int height), {
    let input = document.getElementById(Emval.toValue(id));
    if(input === null){
        input = document.createElement('input');
        input.id = Emval.toValue(id);
        document.body.insertBefore(input, document.getElementById('canvas').nextSibling);
    }
    input.addEventListener(Emval.toValue(event), window[Emval.toValue(callback)], true);
    input.type = Emval.toValue(type);
    input.style.position = 'fixed';
    input.style.left = pos_x + 'px';
    input.style.top = pos_y + 'px';
    input.style.width = width + 'px';
    input.style.height = height + 'px';
    input.style.opacity = 0;
});
EM_JS(void, destroy_element, (emscripten::EM_VAL id), {
    let input = document.getElementById(Emval.toValue(id));
    if(input !== null){
        input.remove();
    }
});
EM_ASYNC_JS(void, destroy_element_async, (emscripten::EM_VAL id, int delay_ms), {
    let input = document.getElementById(Emval.toValue(id));
    if(input !== null){
        input.remove();
    }
});

void ImGui::Ext::CreateHTMLButton(const char *anID, const char *anEvent, const char *aJSFunctonName)
{
    create_button(emscripten::val(anID).as_handle(), emscripten::val(anEvent).as_handle(), emscripten::val(aJSFunctonName).as_handle(), (int)ImGui::GetCursorPosX(), (int)ImGui::GetCursorPosY(), (int)ImGui::GetItemRectSize().x, (int)ImGui::GetItemRectSize().y);
}

void ImGui::Ext::CreateHTMLButton(ImVec2 aPosition, ImVec2 aSize, const char *anID, const char *anEvent, const char *aJSFunctonName)
{
    create_button(emscripten::val(anID).as_handle(), emscripten::val(anEvent).as_handle(), emscripten::val(aJSFunctonName).as_handle(), (int)aPosition.x, (int)aPosition.y, (int)aSize.x, (int)aSize.y);
}

void ImGui::Ext::CreateHTMLInput(const char *anID, const char *aType, const char *anEvent, const char *aJSFunctonName)
{
    create_input(emscripten::val(anID).as_handle(), emscripten::val(aType).as_handle(), emscripten::val(anEvent).as_handle(), emscripten::val(aJSFunctonName).as_handle(), (int)ImGui::GetCursorPosX(), (int)ImGui::GetCursorPosY(), (int)ImGui::GetItemRectSize().x, (int)ImGui::GetItemRectSize().y);
}

void ImGui::Ext::CreateHTMLInput(ImVec2 aPosition, ImVec2 aSize, const char *anID, const char *aType, const char *anEvent, const char *aJSFunctonName)
{
    create_input(emscripten::val(anID).as_handle(), emscripten::val(aType).as_handle(), emscripten::val(anEvent).as_handle(), emscripten::val(aJSFunctonName).as_handle(), (int)aPosition.x, (int)aPosition.y, (int)aSize.x, (int)aSize.y);
}

void ImGui::Ext::DestroyHTMLElement(const char *anID, int aDelayMillis)
{
    if(aDelayMillis > 0)
    {
        destroy_element_async(emscripten::val(anID).as_handle(), aDelayMillis);
    }
    else
    {
        destroy_element(emscripten::val(anID).as_handle());
    }
}

bool ImGui::Ext::TimedSyllable(std::string aValue, uint aStartTime, uint anEndTime, uint aCurrentTime, bool aShowProgress)
{
    ImVec2 size = CalcTextSize(aValue.data());
    ImVec2 pos = GetCursorScreenPos();
    float start = aStartTime;
    float end = anEndTime;
    ImVec2 timeStartPos = {pos.x, pos.y + (size.y * 1.2f)};
    ImVec2 timeEndPos = {remap(clamp(aCurrentTime, start, end), start, end, pos.x, pos.x + size.x), pos.y + (size.y * 1.2f)};
    Text(aValue.data());
    bool clicked = IsItemClicked(0);
    if(aShowProgress)
    {
        ImDrawList* drawList = GetWindowDrawList();
        drawList->AddLine(timeStartPos, timeEndPos, IM_COL32_WHITE, 2);
        drawList->AddTriangleFilled(timeStartPos, {timeStartPos.x + 5, timeStartPos.y}, {timeStartPos.x, timeStartPos.y + 5}, IM_COL32_WHITE);
    }
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
