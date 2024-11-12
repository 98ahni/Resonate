//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include <string>
#include <vector>
#include <unordered_map>

class Gamepad
{
public:
    enum Mapping
    {
        Xinput,
        DualShock3, DualShock4, DualSense, PSClassic,
        SwitchPro, JoyConPair, JoyConL, JoyConR,
        Standard, Other
    };
    enum Button
    {
        std0, std1, std2, std3, std4, std5, std6, std7, std8, std9, std10, std11, std12, std13, std14, std15, std16,
        A=std0, B, X, Y, LB, RB, LT, RT, Back, Start, LSB, RSB, D_Up, D_Down, D_Left, D_Right, Guide,
        Cross=std0, Circle, Square, Triangle, L1, R1, L2, R2, Share, Option, L3, R3, PS=std16, TouchPadClick,
        L=std4, R, ZL, ZR, Minus, Plus, LeftStick, RightStick, Home=std16, Capture, LeftSL, LeftSR, RightSL, RightSR
    };
    enum Axis
    {
        std0X, std0Y, std1X, std1Y,
        LeftStickX=std0X, LeftStickY, RightStickX, RightStickY,
        StickX=std0X, StickY
    };
    static void Initialize();
    static void Update();
    static Mapping GetMapping(int aControllerID);
    static int GetCount();
    static std::vector<int> GetConnectedIDs();
    static bool GetButton(int aControllerID, Button aButton);
    static bool GetButtonDown(int aControllerID, Button aButton);
    static bool GetButtonUp(int aControllerID, Button aButton);
    static float GetButtonAnalog(int aControllerID, Button aButton);
    static float GetButtonAnalogRaw(int aControllerID, Button aButton);
    static float GetAxis(int aControllerID, Axis anAxis);
    static float GetAxisRaw(int aControllerID, Axis anAxis);
    static float GetAxisDeltaRaw(int aControllerID, Axis anAxis);
    static bool GetAxisCrossedDeadZone(int aControllerID, Axis anAxis);
    static float GetTimeSinceToggled(int aControllerID, Button aButton);
    static float GetTimeSinceCrossedDeadZone(int aControllerID, Axis anAxis);
    static bool GetButton(Button aButton);
    static bool GetButtonDown(Button aButton);
    static bool GetButtonUp(Button aButton);
    static float GetButtonAnalog(Button aButton);
    static float GetButtonAnalogRaw(Button aButton);
    static float GetAxis(Axis anAxis);
    static float GetAxisRaw(Axis anAxis);
    static float GetAxisDeltaRaw(Axis anAxis);
    static bool GetAxisCrossedDeadZone(Axis anAxis);
    static float GetTimeSinceToggled(Button aButton);
    static float GetTimeSinceCrossedDeadZone(Axis anAxis);
    static int GetControllerPressing(Button aButton);
    static int GetControllerWithLastEvent();
    static void SetDeadZone(float aValue);
    static void UseVerticalJoyCon();
    static void UseHorizontalJoyCon();

private:
    struct ButtonState
    {
        bool myIsDown;
        float myValue;
        bool myIsInitial;
        float myTimeOfLastToggle;
    };
    struct AxisState
    {
        float myDelta;
        float myValue;
        bool myIsInitial;
        float myTimeOfLastToggle;
    };
    struct State
    {
        int myID;
        std::string myName;
        Mapping myMapping;
        float myTime;
        float myTimeOfLastToggle;
        std::vector<ButtonState> myButtons;
        std::vector<AxisState> myAxes;
    };
    static inline std::unordered_map<int, State> myGamepads;
    inline static float myDeadZone = 0;
    inline static bool myUseJoyConVertical = false;

    static Button RemapButtonToStd(Button aButton, Mapping aMapping);
};