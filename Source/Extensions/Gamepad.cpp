//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#define private public
#include "Gamepad.h"
#undef private
#include <float.h>
#include <emscripten.h>
#include <emscripten/val.h>
#include <stdio.h>
#include <Defines.h>

EM_JS(void, initialize_gamepad_events, (), {
    window.addEventListener("gamepadconnected", (e)=>{
        _jsGamepadConnected(Emval.toHandle({
            index: e.gamepad.index,
            name: e.gamepad.id,
            standardMap: e.gamepad.mapping == 'standard',
            buttonCount: e.gamepad.buttons.length,
            axesCount: e.gamepad.axes.length,
            timeStamp: e.gamepad.timestamp
        }));
    });
    window.addEventListener("gamepaddisconnected", (e)=>{_jsGamepadDisconnected(e.gamepad.index)});
});
extern"C" EMSCRIPTEN_KEEPALIVE void jsGamepadConnected(emscripten::EM_VAL aGamepad)
{
    emscripten::val gamepad = VAR_FROM_JS(aGamepad);
    int index = gamepad["index"].as<int>();
    std::string name = gamepad["name"].as<std::string>();
    Gamepad::myGamepads[index] = {index, name};
    Gamepad::myGamepads[index].myMapping = 
        name.contains("XInput") || name.contains("Xbox") ? Gamepad::Xinput : (
            name.contains("054c") ? (name.contains("09cc") || name.contains("0ba0") || name.contains("05c4") ? Gamepad::DualShock3 : (name.contains("0df2") || name.contains("0ce6") ? Gamepad::DualSense : (name.contains("0268") ? Gamepad::DualShock3 : Gamepad::PSClassic))) : (
                name.contains("057e") ? (name.contains("2009") ? Gamepad::SwitchPro : (name.contains("200e") ? Gamepad::JoyConPair : (name.contains("2006") ? Gamepad::JoyConL : (name.contains("2007") ? Gamepad::JoyConR : Gamepad::Other)))) : (
                    gamepad["standardMap"].as<bool>() ? Gamepad::Standard : Gamepad::Other
                )
            )
        );
    Gamepad::myGamepads[index].myTime = gamepad["timeStamp"].as<double>();
    Gamepad::myGamepads[index].myButtons.resize(gamepad["buttonCount"].as<int>(), {false, 0, false, (float)gamepad["timeStamp"].as<double>()});
    Gamepad::myGamepads[index].myAxes.resize(gamepad["axesCount"].as<int>(), {0, 0, false, (float)gamepad["timeStamp"].as<double>()});
    printf("Gamepad %s connected at %i with %i buttons and %i axes.\n", name.data(), index, gamepad["buttonCount"].as<int>(), gamepad["axesCount"].as<int>());
}
extern"C" EMSCRIPTEN_KEEPALIVE void jsGamepadDisconnected(int anIndex)
{
    Gamepad::myGamepads.erase(anIndex);
}

EM_JS(emscripten::EM_VAL, update_gamepad_state, (int index), {
    const gp = navigator.getGamepads()[index];
    let output = {buttons: [], axes: [], timeStamp: gp.timestamp};
    if(gp === null) { return Emval.toHandle(null); }
    gp.buttons.forEach(b => output.buttons.push({digital: b.pressed, analog: b.value}));
    gp.axes.forEach(v => output.axes.push(v));
    return Emval.toHandle(output);
});

void Gamepad::Initialize()
{
    initialize_gamepad_events();
}

void Gamepad::Update()
{
    for(auto&[index, con] : myGamepads)
    {
        emscripten::val state = VAR_FROM_JS(update_gamepad_state(index));
        if(state.isNull()) { continue; }
        int buttonsLen = state["buttons"]["length"].as<int>();
        int axesLen = state["axes"]["length"].as<int>();
        con.myTime = state["timeStamp"].as<float>();
        for(int i = 0; i < buttonsLen; i++)
        {
            bool isDown = state["buttons"][i]["digital"].as<bool>();
            if(con.myButtons[i].myIsInitial) {con.myButtons[i].myTimeSinceToggled = state["timeStamp"].as<double>();}
            con.myButtons[i].myIsInitial = con.myButtons[i].myIsDown != isDown;
            con.myButtons[i].myIsDown = isDown;
            con.myButtons[i].myValue = state["buttons"][i]["analog"].as<double>();
        }
        for(int i = 0; i < axesLen; i++)
        {
            float val = state["axes"][i].as<float>();
            if(con.myAxes[i].myIsInitial) {
                con.myAxes[i].myTimeSinceToggled = state["timeStamp"].as<double>();}
            con.myAxes[i].myIsInitial = (abs(con.myAxes[i].myValue) < myDeadZone) != (abs(val) < myDeadZone);
            con.myAxes[i].myDelta = val - con.myAxes[i].myValue;
            con.myAxes[i].myValue = val;
        }
    }
}

Gamepad::Mapping Gamepad::GetMapping(int aControllerID)
{
    return myGamepads[aControllerID].myMapping;
}

bool Gamepad::GetButton(int aControllerID, Button aButton)
{
    Button button = RemapButtonToStd(aButton, myGamepads[aControllerID].myMapping);
    if(myGamepads[aControllerID].myButtons.size() <= button) { return false; }
    return myGamepads[aControllerID].myButtons[button].myIsDown;
}

bool Gamepad::GetButtonDown(int aControllerID, Button aButton)
{
    Button button = RemapButtonToStd(aButton, myGamepads[aControllerID].myMapping);
    if(myGamepads[aControllerID].myButtons.size() <= button) { return false; }
    return myGamepads[aControllerID].myButtons[button].myIsDown && myGamepads[aControllerID].myButtons[button].myIsInitial;
}

bool Gamepad::GetButtonUp(int aControllerID, Button aButton)
{
    Button button = RemapButtonToStd(aButton, myGamepads[aControllerID].myMapping);
    if(myGamepads[aControllerID].myButtons.size() <= button) { return false; }
    return !myGamepads[aControllerID].myButtons[button].myIsDown && myGamepads[aControllerID].myButtons[button].myIsInitial;
}

float Gamepad::GetButtonAnalog(int aControllerID, Button aButton)
{
    float value = GetButtonAnalogRaw(aControllerID, aButton);
    bool isNegative = value < 0;
    value = remap(abs(value), myDeadZone, 1, 0, 1);
    return (value < 0 ? 0 : value) * (isNegative ? -1 : 1);
}

float Gamepad::GetButtonAnalogRaw(int aControllerID, Button aButton)
{
    Button button = RemapButtonToStd(aButton, myGamepads[aControllerID].myMapping);
    if(myGamepads[aControllerID].myButtons.size() <= button) { return 0; }
    return myGamepads[aControllerID].myButtons[button].myValue;
}

float Gamepad::GetAxis(int aControllerID, Axis anAxis)
{
    float value = GetAxisRaw(aControllerID, anAxis);
    bool isNegative = value < 0;
    value = remap(abs(value), myDeadZone, 1, 0, 1);
    return (value < 0 ? 0 : value) * (isNegative ? -1 : 1);
}

float Gamepad::GetAxisRaw(int aControllerID, Axis anAxis)
{
    if(myGamepads[aControllerID].myAxes.size() <= anAxis) { return 0; }
    return myGamepads[aControllerID].myAxes[anAxis].myValue * (anAxis % 2 == 1 ? -1 : 1);
}

float Gamepad::GetAxisDeltaRaw(int aControllerID, Axis anAxis)
{
    if(myGamepads[aControllerID].myAxes.size() <= anAxis) { return 0; }
    return myGamepads[aControllerID].myAxes[anAxis].myDelta * (anAxis % 2 == 1 ? -1 : 1);
}

bool Gamepad::GetAxisCrossedDeadZone(int aControllerID, Axis anAxis)
{
    if(myGamepads[aControllerID].myAxes.size() <= anAxis) { return 0; }
    return myGamepads[aControllerID].myAxes[anAxis].myIsInitial;
}

float Gamepad::GetTimeSinceToggled(int aControllerID, Button aButton)
{
    Button button = RemapButtonToStd(aButton, myGamepads[aControllerID].myMapping);
    if(myGamepads[aControllerID].myButtons.size() <= button) { return 0; }
    return (myGamepads[aControllerID].myTime - myGamepads[aControllerID].myButtons[button].myTimeSinceToggled) * .001f;
}

float Gamepad::GetTimeSinceCrossedDeadZone(int aControllerID, Axis anAxis)
{
    if(myGamepads[aControllerID].myAxes.size() <= anAxis) { return 0; }
    return (myGamepads[aControllerID].myTime - myGamepads[aControllerID].myAxes[anAxis].myTimeSinceToggled) * .001f;
}

bool Gamepad::GetButton(Button aButton)
{
    for(auto&[index, con] : myGamepads)
    {
        Button button = RemapButtonToStd(aButton, con.myMapping);
        if(con.myButtons.size() <= button) { continue; }
        if(con.myButtons[button].myIsDown) return true;
    }
    return false;
}

bool Gamepad::GetButtonDown(Button aButton)
{
    for(auto&[index, con] : myGamepads)
    {
        Button button = RemapButtonToStd(aButton, con.myMapping);
        if(con.myButtons.size() <= button) { continue; }
        if(con.myButtons[button].myIsDown && con.myButtons[button].myIsInitial) return true;
    }
    return false;
}

bool Gamepad::GetButtonUp(Button aButton)
{
    for(auto&[index, con] : myGamepads)
    {
        Button button = RemapButtonToStd(aButton, con.myMapping);
        if(con.myButtons.size() <= button) { continue; }
        if(!con.myButtons[button].myIsDown && con.myButtons[button].myIsInitial) return true;
    }
    return false;
}

float Gamepad::GetButtonAnalog(Button aButton)
{
    float value = GetButtonAnalog(aButton);
    bool isNegative = value < 0;
    value = remap(abs(value), myDeadZone, 1, 0, 1);
    return (value < 0 ? 0 : value) * (isNegative ? -1 : 1);
}

float Gamepad::GetButtonAnalogRaw(Button aButton)
{
    float max = 0;
    for(auto&[index, con] : myGamepads)
    {
        Button button = RemapButtonToStd(aButton, con.myMapping);
        if(con.myButtons.size() <= button) { continue; }
        if(max < con.myButtons[button].myValue) max = con.myButtons[button].myValue;
    }
    return max;
}

float Gamepad::GetAxis(Axis anAxis)
{
    float value = GetAxisRaw(anAxis);
    bool isNegative = value < 0;
    value = remap(abs(value), myDeadZone, 1, 0, 1);
    return (value < 0 ? 0 : value) * (isNegative ? -1 : 1);
}

float Gamepad::GetAxisRaw(Axis anAxis)
{
    float max = 0;
    for(auto&[index, con] : myGamepads)
    {
        if(con.myAxes.size() <= anAxis) { continue; }
        if(abs(max) < abs(con.myAxes[anAxis].myValue)) max = con.myAxes[anAxis].myValue;
    }
    return max * (anAxis % 2 == 1 ? -1 : 1);
}

float Gamepad::GetAxisDeltaRaw(Axis anAxis)
{
    float max = 0;
    for(auto&[index, con] : myGamepads)
    {
        if(con.myAxes.size() <= anAxis) { continue; }
        if(abs(max) < abs(con.myAxes[anAxis].myDelta)) max = con.myAxes[anAxis].myDelta;
    }
    return max * (anAxis % 2 == 1 ? -1 : 1);
}

bool Gamepad::GetAxisCrossedDeadZone(Axis anAxis)
{
    for(auto&[index, con] : myGamepads)
    {
        if(con.myAxes.size() <= anAxis) { continue; }
        if(con.myAxes[anAxis].myIsInitial) return true;
    }
    return false;
}

float Gamepad::GetTimeSinceToggled(Button aButton)
{
    float min = FLT_MAX;
    for(auto&[index, con] : myGamepads)
    {
        Button button = RemapButtonToStd(aButton, con.myMapping);
        if(con.myButtons.size() <= button) { continue; }
        float time = (con.myTime - con.myButtons[button].myTimeSinceToggled) * .001f;
        if(min > time) min = time;
    }
    return min;
}

float Gamepad::GetTimeSinceCrossedDeadZone(Axis anAxis)
{
    float min = FLT_MAX;
    for(auto&[index, con] : myGamepads)
    {
        if(con.myAxes.size() <= anAxis) { continue; }
        float time = (con.myTime - con.myAxes[anAxis].myTimeSinceToggled) * .001f;
        if(min > time) min = time;
    }
    return min;
}

int Gamepad::GetControllerPressing(Button aButton)
{
    for(auto&[index, con] : myGamepads)
    {
        Button button = RemapButtonToStd(aButton, con.myMapping);
        if(con.myButtons.size() <= button) { continue; }
        if(con.myButtons[button].myIsDown) return index;
    }
    return -1;
}

void Gamepad::SetDeadZone(float aValue)
{
    myDeadZone = aValue;
}

void Gamepad::UseVerticalJoyCon()
{
    myUseJoyConVertical = true;
}

void Gamepad::UseHorizontalJoyCon()
{
    myUseJoyConVertical = false;
}

Gamepad::Button Gamepad::RemapButtonToStd(Button aButton, Mapping aMapping)
{
    if(aMapping == SwitchPro || aMapping == JoyConPair || (!myUseJoyConVertical && (aMapping == JoyConL || aMapping == JoyConR)))
    {
        switch (aButton)
        {
        case A:
            return std1;
        case B:
            return std0;
        case X:
            return std3;
        case Y:
            return std2;
        }
    }
    if(myUseJoyConVertical && aMapping == JoyConL)
    {
        switch (aButton)
        {
        case D_Down:
            return std1;
        case D_Left:
            return std0;
        case D_Up:
            return std3;
        case D_Right:
            return std2;
        }
    }
    if(myUseJoyConVertical && aMapping == JoyConR)
    {
        switch (aButton)
        {
        case X:
            return std1;
        case A:
            return std0;
        case Y:
            return std3;
        case B:
            return std2;
        }
    }
    if(aMapping == JoyConL)
    {
        switch (aButton)
        {
        case LeftStick:
            return std10;
        case L:
            return std8;
        case ZL:
            return std6;
        case Minus:
            return std9;
        case Capture:
            return std16;
        case LeftSL:
            return std4;
        case LeftSR:
            return std5;
        default:
            return (Button)INT_MAX;
        }
    }
    if(aMapping == JoyConR)
    {
        switch (aButton)
        {
        case RightStick:
            return std10;
        case R:
            return std8;
        case ZR:
            return std7;
        case Plus:
            return std9;
        case Home:
            return std16;
        case RightSL:
            return std4;
        case RightSR:
            return std5;
        default:
            return (Button)INT_MAX;
        }
    }
    return aButton;
}
