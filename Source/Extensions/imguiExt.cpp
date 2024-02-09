#include "imguiExt.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <Serialization/KaraokeData.h>
#include <Defines.h>

extern"C" EMSCRIPTEN_KEEPALIVE void ExecCallback(ImGui::Ext::HTMLEvent aCallback)
{
    aCallback();
}
EM_JS(void, create_button, (emscripten::val id, emscripten::val event, emscripten::val callback, int pos_x, int pos_y, int width, int height), {
    let btn = document.getElementById(id);
    if(btn != 'undefined'){
        btn = document.createElement('button');
        btn.id = id;
        document.body.insertBefore(btn, document.getElementById('canvas').nextSibling);
    }
    if(typeof callback == 'string'){
        btn.addEventListener(event, window[callback], true);
    }else{
        btn.addEventListener(event, _ExecCallback(callback), true);
    }
    btn.style.position = 'fixed';
    btn.style.left = pos_x + 'px';
    btn.style.top = pos_y + 'px';
    btn.style.width = width + 'px';
    btn.style.height = height + 'px';
    btn.style.opacity = 0;
});
EM_JS(void, create_input, (emscripten::val id, emscripten::val type, emscripten::val event, emscripten::val callback, int pos_x, int pos_y, int width, int height), {
    let input = document.getElementById(id);
    if(input != 'undefined'){
        input = document.createElement('input');
        input.id = id;
        document.body.insertBefore(input, document.getElementById('canvas').nextSibling);
    }
    if(typeof callback == 'string'){
        input.addEventListener(event, window[callback], true);
    }else{
        input.addEventListener(event, _ExecCallback(callback), true);
    }
    input.type = type;
    input.style.position = 'fixed';
    input.style.left = pos_x + 'px';
    input.style.top = pos_y + 'px';
    input.style.width = width + 'px';
    input.style.height = height + 'px';
    input.style.opacity = 0;
});
EM_JS(void, destroy_element, (emscripten::val id), {
    let input = document.getElementById(id);
    if(input != 'undefined'){
        input = document.createElement('input');
        input.id = id;
        document.body.insertBefore(input, document.getElementById('canvas').nextSibling);
    }
});
EM_ASYNC_JS(void, destroy_element_async, (emscripten::val id, int delay_ms), {
    let input = document.getElementById(id);
    if(input != 'undefined'){
        input = document.createElement('input');
        input.id = id;
        document.body.insertBefore(input, document.getElementById('canvas').nextSibling);
    }
});

void ImGui::Ext::CreateHTMLButton(const char *anID, const char *anEvent, const char *aJSFunctonName)
{
    create_button(emscripten::val(anID), emscripten::val(anEvent), emscripten::val(aJSFunctonName), (int)ImGui::GetCursorPosX(), (int)ImGui::GetCursorPosY(), (int)ImGui::GetItemRectSize().x, (int)ImGui::GetItemRectSize().y);
}

void ImGui::Ext::CreateHTMLButton(const char *anID, const char *anEvent, HTMLEvent aCallback)
{
    create_button(emscripten::val(anID), emscripten::val(anEvent), emscripten::val((unsigned)aCallback), (int)ImGui::GetCursorPosX(), (int)ImGui::GetCursorPosY(), (int)ImGui::GetItemRectSize().x, (int)ImGui::GetItemRectSize().y);
}

void ImGui::Ext::CreateHTMLButton(ImVec2 aPosition, ImVec2 aSize, const char *anID, const char *anEvent, const char *aJSFunctonName)
{
    create_button(emscripten::val(anID), emscripten::val(anEvent), emscripten::val(aJSFunctonName), (int)aPosition.x, (int)aPosition.y, (int)aSize.x, (int)aSize.y);
}

void ImGui::Ext::CreateHTMLButton(ImVec2 aPosition, ImVec2 aSize, const char *anID, const char *anEvent, HTMLEvent aCallback)
{
    create_button(emscripten::val(anID), emscripten::val(anEvent), emscripten::val((unsigned)aCallback), (int)aPosition.x, (int)aPosition.y, (int)aSize.x, (int)aSize.y);
}

void ImGui::Ext::CreateHTMLInput(const char *anID, const char *aType, const char *anEvent, const char *aJSFunctonName)
{
    create_input(emscripten::val(anID), emscripten::val(aType), emscripten::val(anEvent), emscripten::val(aJSFunctonName), (int)ImGui::GetCursorPosX(), (int)ImGui::GetCursorPosY(), (int)ImGui::GetItemRectSize().x, (int)ImGui::GetItemRectSize().y);
}

void ImGui::Ext::CreateHTMLInput(const char *anID, const char *aType, const char *anEvent, HTMLEvent aCallback)
{
    create_input(emscripten::val(anID), emscripten::val(aType), emscripten::val(anEvent), emscripten::val((unsigned)aCallback), (int)ImGui::GetCursorPosX(), (int)ImGui::GetCursorPosY(), (int)ImGui::GetItemRectSize().x, (int)ImGui::GetItemRectSize().y);
}

void ImGui::Ext::CreateHTMLInput(ImVec2 aPosition, ImVec2 aSize, const char *anID, const char *aType, const char *anEvent, const char *aJSFunctonName)
{
    create_input(emscripten::val(anID), emscripten::val(aType), emscripten::val(anEvent), emscripten::val(aJSFunctonName), (int)aPosition.x, (int)aPosition.y, (int)aSize.x, (int)aSize.y);
}

void ImGui::Ext::CreateHTMLInput(ImVec2 aPosition, ImVec2 aSize, const char *anID, const char *aType, const char *anEvent, HTMLEvent aCallback)
{
    create_input(emscripten::val(anID), emscripten::val(aType), emscripten::val(anEvent), emscripten::val((unsigned)aCallback), (int)aPosition.x, (int)aPosition.y, (int)aSize.x, (int)aSize.y);
}

void ImGui::Ext::DestroyHTMLElement(const char *anID, int aDelayMillis)
{
    if(aDelayMillis > 0)
    {
        destroy_element_async(emscripten::val(anID), aDelayMillis);
    }
    else
    {
        destroy_element(emscripten::val(anID));
    }
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
