//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#pragma once
#include <Extensions/Gamepad.h>
#define Gamepad_FlickUP >
#define Gamepad_FlickDOWN <
#define Gamepad_FlickLEFT <
#define Gamepad_FlickRIGHT >
#define Gamepad_Magnitude(stick) (sqrtf(Gamepad::GetAxisRaw(stick##X) * Gamepad::GetAxisRaw(stick##X)) + (Gamepad::GetAxisRaw(stick##Y) * Gamepad::GetAxisRaw(stick##Y)))
#define Gamepad_FlickAxis(axis, dir) (Gamepad::GetAxisRaw(axis) dir 0 && Gamepad::GetAxisCrossedDeadZone(axis) && Gamepad::GetAxisDeltaRaw(axis) dir 0 && Gamepad::GetTimeSinceCrossedDeadZone(axis) > .1f)
#define Gamepad_Spin(stick) ((Gamepad::GetAxisRaw(stick##X) - Gamepad::GetAxisDeltaRaw(stick##X)) * Gamepad::GetAxisRaw(stick##Y) - (Gamepad::GetAxisRaw(stick##Y) - Gamepad::GetAxisDeltaRaw(stick##Y)) * Gamepad::GetAxisRaw(stick##X))
#define Gamepad_Tap(button) (Gamepad::GetButtonUp(button) && Gamepad::GetTimeSinceToggled(button) < 0.5f)
#define Gamepad_Hold(button) (Gamepad::GetButton(button) && Gamepad::GetTimeSinceToggled(button) > 0.5f)
#define Gamepad_RepeatDelayed(button, rate, delay) (Gamepad::GetButtonDown(button) || (Gamepad::GetButtonRepeating(button, rate) && Gamepad::GetTimeSinceToggled(button) > delay))
typedef unsigned int ImU32;
struct ImVec2;
struct ImDrawList;
#ifndef IM_COL32_WHITE
#define IM_COL32_WHITE 0xFFFFFFFF
#endif

namespace HUDSprite
{
    enum HUDSpriteID
    {
        DocColor, PreviewBtn, ShiftTimes, FontSize, Latency,
        MenuBtn, EffectBtn, SyllableBtn, TimeEnd, TimeStart,
        SpeedUp, SpeedDown, LineTagPlus, LineTagMinus, NoEffectBtn, SingerBtn, ImageBtn,
        LineMoveUp, LineMoveDown, LineMergeUp, LineMergeDown, LineDuplicate, LineSplit, LineRemove,
        CaseCapital, CaseMajus, CaseMinus, CaseToggle, MenuToggle,
        StopBtn, PlayBtn, PauseBtn, RW5sBtn, FF5sBtn, SeekToMarkBtn, LayoutBtn, AdjustBtn,
        BtnPadBGPS, DPadBGPS, BtnPadBG, DPadBG, StickSpinBG, StickFlickBG,
        DPadFillPSY, DPadFillPSX, DPadFillY, DPadFillX,
        //DPadFillPSYR, DPadFillPSXR, DPadFillYR, DPadFillXR,       // Might be useful later
        ArrowUpBtn, ArrowDownBtn, ArrowLeftBtn, ArrowRightBtn,
        SpinIcon, BumperFill, TriggerFill, BtnFill
    };
}

void DrawHudSprite(HUDSprite::HUDSpriteID aSprite, ImVec2 aSize, ImU32 aColor = IM_COL32_WHITE);
void AddHudSpriteTo(ImDrawList* aDrawList, HUDSprite::HUDSpriteID aSprite, ImVec2 aScreenPosition, ImVec2 aSize, ImU32 aColor = IM_COL32_WHITE);
void DoGamepadActions();