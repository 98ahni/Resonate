//  This file is licenced under the GNU General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
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
    btn.addEventListener(Emval.toValue(event), window[Emval.toValue(callback)], false);
    btn.style.position = 'fixed';
    btn.style.left = pos_x + 'px';
    btn.style.top = pos_y + 'px';
    btn.style.width = width + 'px';
    btn.style.height = height + 'px';
    btn.style.opacity = 0.1;
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
EM_JS(void, add_window_event, (emscripten::EM_VAL event, emscripten::EM_VAL callback), {
    window.addEventListener(Emval.toValue(event), window[Emval.toValue(callback)], true);
});
EM_JS(void, remove_window_event, (emscripten::EM_VAL event, emscripten::EM_VAL callback), {
    window.removeEventListener(Emval.toValue(event), window[Emval.toValue(callback)], true);
});

void ImGui::Ext::CreateHTMLButton(const char *anID, const char *anEvent, const char *aJSFunctonName)
{
    create_button(emscripten::val(anID).as_handle(), emscripten::val(anEvent).as_handle(), emscripten::val(aJSFunctonName).as_handle(), (int)ImGui::GetItemRectMin().x, (int)ImGui::GetItemRectMin().y, (int)ImGui::GetItemRectSize().x, (int)ImGui::GetItemRectSize().y);
}

void ImGui::Ext::CreateHTMLButton(ImVec2 aPosition, ImVec2 aSize, const char *anID, const char *anEvent, const char *aJSFunctonName)
{
    create_button(emscripten::val(anID).as_handle(), emscripten::val(anEvent).as_handle(), emscripten::val(aJSFunctonName).as_handle(), (int)aPosition.x, (int)aPosition.y, (int)aSize.x, (int)aSize.y);
}

void ImGui::Ext::CreateHTMLInput(const char *anID, const char *aType, const char *anEvent, const char *aJSFunctonName)
{
    create_input(emscripten::val(anID).as_handle(), emscripten::val(aType).as_handle(), emscripten::val(anEvent).as_handle(), emscripten::val(aJSFunctonName).as_handle(), (int)ImGui::GetItemRectMin().x, (int)ImGui::GetItemRectMin().y, (int)ImGui::GetItemRectSize().x, (int)ImGui::GetItemRectSize().y);
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

void ImGui::Ext::AddWindowEvent(const char *anEvent, const char *aJSFunctionName)
{
    add_window_event(VAR_TO_JS(anEvent), VAR_TO_JS(aJSFunctionName));
}

void ImGui::Ext::RemoveWindowEvent(const char *anEvent, const char *aJSFunctionName)
{
    remove_window_event(VAR_TO_JS(anEvent), VAR_TO_JS(aJSFunctionName));
}

EM_ASYNC_JS(emscripten::EM_VAL, get_clipboard_content, (), {
	//const output = await new Promise((resolve)=>{navigator.clipboard.readText().then((text)=>{resolve(text);});});
	var output = '';
	const clipboardContents = await navigator.clipboard.read();
    for (const item of clipboardContents) {
		if (item.types.includes("text/plain")) {
    		let blob = await item.getType("text/plain");
    		output = await blob.text();
			console.log(output);
			//return Emval.toHandle(output);
		}
	}
	return Emval.toHandle(output);
});
extern"C" EMSCRIPTEN_KEEPALIVE void GetClipboardContent()
{
	static std::string output = "";
	output = VAR_FROM_JS(get_clipboard_content()).as<std::string>().c_str();
	printf("Pasting '%s'\n", output.data());
    if(ImGui::GetIO().WantTextInput)
    {
        ImGui::GetIO().AddInputCharactersUTF8(output.data());
    }
}

EM_ASYNC_JS(void, set_clipboard_content, (emscripten::EM_VAL content), {
	const type = "text/plain";
  	const blob = new Blob([Emval.toValue(content)], { type });
  	const data = [new ClipboardItem({ [type]: blob })];
  	await navigator.clipboard.write(data);
});
void SetClipboardContent(bool aShouldCut)
{
    ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetActiveID());
    if(ImGui::GetIO().WantTextInput && state)
    {
        const char* content = std::string(state->TextA.Data).substr(state->GetSelectionStart(), state->GetSelectionEnd()).data();
	    set_clipboard_content(VAR_TO_JS(content));
        if(aShouldCut)
        {
            state->ClearSelection();
        }
    }
}
extern"C" EMSCRIPTEN_KEEPALIVE void CopyClipboardContent()
{
    SetClipboardContent(false);
}
extern"C" EMSCRIPTEN_KEEPALIVE void CutClipboardContent()
{
    SetClipboardContent(true);
}

void ImGui::Ext::SetShortcutEvents()
{
    AddWindowEvent("copy", "_CopyClipboardContent");
    AddWindowEvent("cut", "_CutClipboardContent");
    AddWindowEvent("paste", "_GetClipboardContent");
}

bool ImGui::Ext::TimedSyllable(std::string aValue, uint aStartTime, uint anEndTime, uint aCurrentTime, bool aShowProgress)
{
    ImVec2 size = CalcTextSize(aValue.data());
    ImVec2 pos = GetCursorScreenPos();
    float start = aStartTime;
    float end = anEndTime;
    ImVec2 timeStartPos = {pos.x, pos.y + (size.y * 1.2f)};
    ImVec2 timeEndPos = {remap(clamp(aCurrentTime, start, end), start, end, pos.x, pos.x + size.x), pos.y + (size.y * 1.2f)};
    if(aCurrentTime < start)
    {
        uint startCol = Serialization::KaraokeDocument::Get().GetStartColor();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_FROM_DOC(startCol) | 0xFF000000);
    }
    else
    {
        uint endCol = Serialization::KaraokeDocument::Get().GetEndColor();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_FROM_DOC(endCol) | 0xFF000000);
    }
    Text(aValue.data());
    ImGui::PopStyleColor();
    bool clicked = IsItemClicked(0);
    if(aShowProgress)
    {
        ImDrawList* drawList = GetWindowDrawList();
        if(aValue.empty() || aValue == " ")
        {
            drawList->AddTriangleFilled({size.x + timeStartPos.x - 1, timeStartPos.y + 1}, {size.x + timeStartPos.x - 5, timeStartPos.y + 5}, {size.x + timeStartPos.x - 1, timeStartPos.y + 5}, IM_COL32(255, 200, 255, 255));
        }
        else
        {
            drawList->AddTriangleFilled(timeStartPos, {timeStartPos.x + 5, timeStartPos.y + 5}, {timeStartPos.x, timeStartPos.y + 5}, IM_COL32_WHITE);
            drawList->AddLine(timeStartPos, timeEndPos, IM_COL32_WHITE, 2);
        }
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

bool ImGui::Ext::ToggleSwitch(const char *aLabel, bool *aValue)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(aLabel);
    const ImVec2 label_size = CalcTextSize(aLabel, NULL, true);

    const float square_sz = GetFrameHeight();
    const ImVec2 size = ImVec2(square_sz * 2.5f, square_sz);
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, pos + ImVec2(size.x + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *aValue = !(*aValue);
        MarkItemEdited(id);
    }

    const ImRect check_bb(pos, pos + size);
    RenderNavHighlight(total_bb, id);
    RenderFrame(check_bb.Min + style.ItemInnerSpacing, check_bb.Max - style.ItemInnerSpacing, GetColorU32(*aValue ? ImGuiCol_CheckMark : ImGuiCol_FrameBg), true, style.FrameRounding);
    ImU32 check_col = GetColorU32(ImGuiCol_CheckMark);
    bool mixed_value = (g.LastItemData.InFlags & ImGuiItemFlags_MixedValue) != 0;
    ImRect pad;
    if (mixed_value)
    {
        // Undocumented tristate/mixed/indeterminate checkbox (#2644)
        // This may seem awkwardly designed because the aim is to make ImGuiItemFlags_MixedValue supported by all widgets (not just checkbox)
        pad = ImRect((size.x - square_sz) * .5f, 0, (size.x - square_sz) * .5f, 0);
        //window->DrawList->AddCircleFilled(check_bb.GetCenter(), square_sz * .5f, GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
    }
    else if (*aValue)
    {
        //const float pad = ImMax(1.0f, IM_TRUNC(square_sz / 6.0f));
        //RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad * 2.0f);
        pad = ImRect(size.x - square_sz, 0, 0, 0);
    }
    else
    {
        pad = ImRect(0, 0, size.x - square_sz, 0);
    }
    window->DrawList->AddRectFilled(check_bb.Min + pad.Min, check_bb.Max - pad.Max, GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button), style.FrameRounding);

    ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
    if (g.LogEnabled)
        LogRenderedText(&label_pos, mixed_value ? "[~]" : *aValue ? "[x]" : "[ ]");
    if (label_size.x > 0.0f)
        RenderText(label_pos, aLabel);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, aLabel, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*aValue ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}

bool ImGui::Ext::TabMenu(ImVector<std::string> someLabels, int *aValue)
{
    bool output = false;
    for(int i = 0; i < someLabels.size(); i++)
    {
        bool isSelected = *aValue == i;
        if(isSelected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, GetColorU32(ImGuiCol_ButtonActive));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GetColorU32(ImGuiCol_ButtonActive));
        }
        if(ImGui::Button(someLabels[i].data()))
        {
            output = true;
            *aValue = i;
        }
        if(isSelected)
        {
            ImGui::PopStyleColor(2);
        }
    }
}
