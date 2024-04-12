#include "TouchInput.h"
#include <emscripten.h>
#include <stdio.h>
#include <unordered_map>
#include <imgui.h>
#include <imgui_internal.h>

std::unordered_map<int, ImVec2> g_touches;
ImVector<ImVec2> g_inputQueue;
int g_mainTouch = -1;
int g_subTouch = -1;
bool g_usingKeyboard = false;

EM_JS(void, force_click_event, (void* node), {
	try
	{
		node.dispatchEvent(new MouseEvent('click'));
	}
	catch(e)
	{
		var evt = document.createEvent('MouseEvents');
		evt.initMouseEvent('click', true, false, window, 0, 0, 0, 80,
			20, false, false, false, false, 0, null);
		node.dispatchEvent(evt);
	}
});

EM_JS(bool, has_physical_touch, (), {
	return window.matchMedia('(any-pointer: coarse)').matches;
});

EM_ASYNC_JS(void, show_touch_keyboard, (bool is_num_board, int pos_y), {
	let input = document.createElement('input');
	input.id = 'temp-text-input';
	if(is_num_board)
	{
		input.type = 'number';
	}
	else
	{
		input.type = 'text';
	}
	input.addEventListener('input', (evt) => {
		if(evt.inputType == "deleteContentBackward")
		{
			_TouchExtraKeyEvents(0, true);
			evt.stopPropagation();
			setTimeout(()=>{_TouchExtraKeyEvents(0, false); }, 60);
		}
		if(evt.inputType == "deleteContentForeward")
		{
			_TouchExtraKeyEvents(1, true);
			evt.stopPropagation();
			setTimeout(()=>{_TouchExtraKeyEvents(1, false); }, 60);
		}
	});
	input.style.position = 'fixed';
	input.style.left = '-1000px';
	input.style.top = pos_y + 'px';
	document.body.insertBefore(input, document.getElementById('canvas'));
	//input.addEventListener('click', (evt) => {
	//	let field = document.getElementById('temp-text-input');
	//	field.style.visibility = 'visible';
	//	field.focus({ preventScroll: true });
	//	//field.style.visibility = 'hidden';
	//	console.log('Input Clicked!');
	//});
	//force_click_event(input);
	//input.focus({preventScroll: true});
});

EM_JS(void, always_show_touch_keyboard, (), {
	let input = document.createElement('input');
	input.id = 'mobile-text-input';
	input.type = 'text';
	input.addEventListener('focusout', (evt) =>
	{
		alert('Focus lost!');
	});
	document.body.insertBefore(input, document.getElementById('canvas'));
});

EM_JS(void, hide_touch_keyboard, (), {
	let input = document.getElementById('temp-text-input');
	input.remove();
});

EM_JS(void, touch_input_handler, (), {
	const el = document.getElementById('canvas');
	el.addEventListener('touchstart', (evt) => {
		//_jsPrepPlayback();
		for(var i = 0; i < evt.changedTouches.length; ++i)
		{
			var touch = evt.changedTouches[i];
			_TouchStart(touch.identifier, touch.clientX, touch.clientY);
		}
		evt.preventDefault();
	});
	el.addEventListener('touchend', (evt) => {
		for(var i = 0; i < evt.changedTouches.length; ++i)
		{
			var touch = evt.changedTouches[i];
			_TouchEnd(touch.identifier, touch.clientX, touch.clientY);
		}
		evt.preventDefault();
	});
	el.addEventListener('touchcancel', (evt) => {
		for(var i = 0; i < evt.changedTouches.length; ++i)
		{
			var touch = evt.changedTouches[i];
			_TouchCancel(touch.identifier, touch.clientX, touch.clientY);
		}
		evt.preventDefault();
	});
	el.addEventListener('touchmove', (evt) => {
		for(var i = 0; i < evt.changedTouches.length; ++i)
		{
			var touch = evt.changedTouches[i];
			_TouchMove(touch.identifier, touch.clientX, touch.clientY);
		}
		evt.preventDefault();
	});
});
extern "C" EMSCRIPTEN_KEEPALIVE void TouchStart(int ID, double X, double Y)
{
	g_touches[ID] = {(float)X, (float)Y};
	if(g_mainTouch == -1) g_mainTouch = ID;
	else if(g_subTouch == -1) g_subTouch = ID;
	if(g_mainTouch == ID)
	{
		ImGui::GetIO().AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
		ImGui::GetIO().AddMousePosEvent(X, Y);
		ImGui::GetIO().AddMouseButtonEvent(0, true);
		g_inputQueue.push_back({(float)X, (float)Y});
		//EM_ASM(if(!document.getElementById('mobile-text-input')) { always_show_touch_keyboard(); }
		//	document.getElementById('mobile-text-input').focus({preventScroll: true}););
		EM_ASM(if(document.getElementById('temp-text-input')) { 
			document.getElementById('temp-text-input').focus({preventScroll: true});});
		EM_ASM(if(document.getElementById('temp-file-input')) { 
			document.getElementById('temp-file-input').click();});
	}
	else if(g_subTouch == ID)
	{
		ImGui::GetIO().AddMouseButtonEvent(0, false);
	}
}
extern "C" EMSCRIPTEN_KEEPALIVE void TouchEnd(int ID, double X, double Y)
{
	g_touches.erase(ID);
	//EM_ASM(_jsPrepPlayback());
	if(g_mainTouch == ID && g_subTouch == -1)
	{
		g_mainTouch = -1;
		ImGui::GetIO().AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
		ImGui::GetIO().AddMousePosEvent(X, Y);
		ImGui::GetIO().AddMouseButtonEvent(0, false);
		g_inputQueue.push_back({(float)X, (float)Y});
	}
	else if(g_subTouch == ID)
	{
		g_mainTouch = -1;
		g_subTouch = -1;
		ImGui::GetIO().AddKeyAnalogEvent(ImGuiKey_F15, false, -1);
	}
}
extern "C" EMSCRIPTEN_KEEPALIVE void TouchCancel(int ID, double X, double Y)
{
	g_touches.erase(ID);
	if(g_mainTouch == ID && g_subTouch == -1)
	{
		g_mainTouch = -1;
		ImGui::GetIO().AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
		ImGui::GetIO().AddMousePosEvent(X, Y);
		ImGui::GetIO().AddMouseButtonEvent(0, false);
		g_inputQueue.push_back({(float)X, (float)Y});
	}
	else if(g_subTouch == ID)
	{
		g_mainTouch = -1;
		g_subTouch = -1;
		ImGui::GetIO().AddKeyAnalogEvent(ImGuiKey_F15, false, -1);
	}
}
extern "C" EMSCRIPTEN_KEEPALIVE void TouchMove(int ID, double X, double Y)
{
	if(g_subTouch == ID && g_touches.contains(ID))
	{
		ImGui::GetIO().AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
		ImGui::GetIO().AddMouseWheelEvent((float)(X - g_touches[ID].x) * .01f, (float)(Y - g_touches[ID].y) * .01f);
	}
	g_touches[ID] = {(float)X, (float)Y};
	if((g_subTouch == ID && g_mainTouch != -1) || (g_mainTouch == ID && g_subTouch != -1))
	{
		ImGui::GetIO().AddKeyAnalogEvent(ImGuiKey_F15, true, sqrt(
			((g_touches[g_subTouch].x - g_touches[g_mainTouch].x) *
			(g_touches[g_subTouch].x - g_touches[g_mainTouch].x)) +
			((g_touches[g_subTouch].y - g_touches[g_mainTouch].y) *
			(g_touches[g_subTouch].y - g_touches[g_mainTouch].y))
		));
	}
	if(g_mainTouch == ID)
	{
		g_inputQueue.push_back({(float)X, (float)Y});
	}
}
extern "C" EMSCRIPTEN_KEEPALIVE void TouchExtraKeyEvents(int ID, bool Down)
{
	if(ID == 0)
	{
		ImGui::GetIO().AddKeyEvent(ImGuiKey_Backspace, Down);
	}
	if(ID == 1)
	{
		ImGui::GetIO().AddKeyEvent(ImGuiKey_Delete, Down);
	}
}

void TouchInput_Init()
{
	touch_input_handler();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
	ImGui::GetIO().ConfigInputTrickleEventQueue = false;
	ImGui::GetIO().MouseDoubleClickMaxDist = 16.f;
	//always_show_touch_keyboard();
}

void TouchInput_RunInput()
{
	for(int i = 0; i < g_inputQueue.Size; i++)
	{
		ImGui::GetIO().AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
		ImGui::GetIO().AddMousePosEvent(g_inputQueue[i].x, g_inputQueue[i].y);
	}
	g_inputQueue.clear();
	if(g_mainTouch != -1)
	{
		ImGui::GetIO().AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
		ImGui::GetIO().AddMousePosEvent(g_touches[g_mainTouch].x, g_touches[g_mainTouch].y);
	}
}

void TouchInput_ReadyKeyboard(bool isNum)
{
	if(g_usingKeyboard) return;
	if(!(ImGui::IsItemClicked(0) && TouchInput_HasTouch())) return;
	printf("Keyboard Show!\n");
	g_usingKeyboard = true;
	show_touch_keyboard(isNum, ImGui::GetCursorScreenPos().y);
}

void TouchInput_CheckKeyboard()
{
	//if(!g_usingKeyboard)
	//{
	//	printf("Keyboard Show!\n");
	//	g_usingKeyboard = true;
	//	show_touch_keyboard(
	//		(ImGui::GetCurrentContext()->InputTextState.Flags &
	//			(ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsHexadecimal)) != 0
	//	);
	//}
	//else
	if((float)(ImGui::GetCurrentContext()->Time - ImGui::GetIO().MouseClickedTime[0]) > ImGui::GetIO().MouseDoubleClickTime)
	{
		if(!ImGui::GetIO().WantTextInput)
		{
			if(g_usingKeyboard)
			{
				printf("Keyboard Hide!\n");
				g_usingKeyboard = false;
				hide_touch_keyboard();
			}
		}
	}
}

void AddInputCharacter(char c, bool shift)
{
	ImGui::GetIO().AddInputCharacter(shift ? c + 32 : c);
}

void TouchInput_DrawKeyboardWindow()
{
	static bool s_shift = false;
	ImGui::Begin("Virtual Keyboard");
	if(ImGui::Button("1")) { AddInputCharacter('1', s_shift); } ImGui::SameLine();
	if(ImGui::Button("2")) { AddInputCharacter('2', s_shift); } ImGui::SameLine();
	if(ImGui::Button("3")) { AddInputCharacter('3', s_shift); } ImGui::SameLine();
	if(ImGui::Button("4")) { AddInputCharacter('4', s_shift); } ImGui::SameLine();
	if(ImGui::Button("5")) { AddInputCharacter('5', s_shift); } ImGui::SameLine();
	if(ImGui::Button("6")) { AddInputCharacter('6', s_shift); } ImGui::SameLine();
	if(ImGui::Button("7")) { AddInputCharacter('7', s_shift); } ImGui::SameLine();
	if(ImGui::Button("8")) { AddInputCharacter('8', s_shift); } ImGui::SameLine();
	if(ImGui::Button("9")) { AddInputCharacter('9', s_shift); } ImGui::SameLine();
	if(ImGui::Button("0")) { AddInputCharacter('0', s_shift); }
	if(ImGui::Button("Q")) { AddInputCharacter('q', s_shift); } ImGui::SameLine();
	if(ImGui::Button("W")) { AddInputCharacter('w', s_shift); } ImGui::SameLine();
	if(ImGui::Button("E")) { AddInputCharacter('e', s_shift); } ImGui::SameLine();
	if(ImGui::Button("R")) { AddInputCharacter('r', s_shift); } ImGui::SameLine();
	if(ImGui::Button("T")) { AddInputCharacter('t', s_shift); } ImGui::SameLine();
	if(ImGui::Button("Y")) { AddInputCharacter('y', s_shift); } ImGui::SameLine();
	if(ImGui::Button("U")) { AddInputCharacter('u', s_shift); } ImGui::SameLine();
	if(ImGui::Button("I")) { AddInputCharacter('i', s_shift); } ImGui::SameLine();
	if(ImGui::Button("O")) { AddInputCharacter('o', s_shift); } ImGui::SameLine();
	if(ImGui::Button("P")) { AddInputCharacter('p', s_shift); }
	if(ImGui::Button("A")) { AddInputCharacter('a', s_shift); } ImGui::SameLine();
	if(ImGui::Button("S")) { AddInputCharacter('s', s_shift); } ImGui::SameLine();
	if(ImGui::Button("D")) { AddInputCharacter('d', s_shift); } ImGui::SameLine();
	if(ImGui::Button("F")) { AddInputCharacter('f', s_shift); } ImGui::SameLine();
	if(ImGui::Button("G")) { AddInputCharacter('g', s_shift); } ImGui::SameLine();
	if(ImGui::Button("H")) { AddInputCharacter('h', s_shift); } ImGui::SameLine();
	if(ImGui::Button("J")) { AddInputCharacter('j', s_shift); } ImGui::SameLine();
	if(ImGui::Button("K")) { AddInputCharacter('k', s_shift); } ImGui::SameLine();
	if(ImGui::Button("L")) { AddInputCharacter('l', s_shift); }
	if(ImGui::Button("Z")) { AddInputCharacter('z', s_shift); } ImGui::SameLine();
	if(ImGui::Button("X")) { AddInputCharacter('x', s_shift); } ImGui::SameLine();
	if(ImGui::Button("C")) { AddInputCharacter('c', s_shift); } ImGui::SameLine();
	if(ImGui::Button("V")) { AddInputCharacter('v', s_shift); } ImGui::SameLine();
	if(ImGui::Button("B")) { AddInputCharacter('b', s_shift); } ImGui::SameLine();
	if(ImGui::Button("N")) { AddInputCharacter('n', s_shift); } ImGui::SameLine();
	if(ImGui::Button("M")) { AddInputCharacter('m', s_shift); } ImGui::SameLine();
	if(ImGui::Button(",")) { AddInputCharacter(',', s_shift); } ImGui::SameLine();
	if(ImGui::Button(".")) { AddInputCharacter('.', s_shift); } ImGui::SameLine();
	if(ImGui::Button("-")) { AddInputCharacter('-', s_shift); }
	if(ImGui::Button("SHIFT")) { s_shift = true; } ImGui::SameLine();
	if(ImGui::Button("SPACE")) { ImGui::GetIO().AddInputCharacter(' '); }
	ImGui::End();
}

bool TouchInput_HasTouch()
{
	return has_physical_touch();
}
