//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "GamepadActions.h"
#include "Extensions/Gamepad.h"
#include "Extensions/imguiExt.h"
#include "Serialization/KaraokeData.h"
#include "Windows/TimingEditor.h"
#include "Windows/AudioPlayback.h"

#define Gamepad_FlickUP >
#define Gamepad_FlickDOWN <
#define Gamepad_FlickLEFT <
#define Gamepad_FlickRIGHT >
#define Gamepad_FlickAxis(axis, dir) (Gamepad::GetAxisRaw(axis) dir 0 && Gamepad::GetAxisCrossedDeadZone(axis) && Gamepad::GetAxisDeltaRaw(axis) dir 0 && Gamepad::GetTimeSinceCrossedDeadZone(axis) > .1f)

enum Layer
{
    Standard, Settings, Effects, Layout, Adjust, Recover
};
Layer g_layerLastFrame;

void DoGamepadActions()
{
    /*
    R Stick spin - scroll

    Modifiers (affects D-pad, L-stick, [Confirm], [Decline], L, R)
    - ZL
    - ZR
    - [Menu]
    - [Action]
    
    Focus windows
    - Image
    - Latency
    - Change Font Size
    - Doc Colors
    - Shift Timings
    - Singer

    Standard
    - D-pad = arrows
    - [Confirm] - Set start time
    - [Decline] - Set end time
    - L/R - Change speed
    - Audio (L stick)
    - - Click - Stop
    - - Flick Down - Play/Pause
    - - Flick Up - Progress to marker
    - - Flick Left/Right - RW/FF 5s
    
    Quick settings ([Menu])
    - Tap - Hide/Show overlay
    - Hold
    - - [Confirm] - Open/Close Preview
    - - D-pad Up - Open Latency
    - - D-pad Left - Open Change Font Size
    - - D-pad Right - Open Doc Colors
    - - D-pad Down - Open Shift Timings
    - - L-stick spin - Cycle doc/local singers (release to open Singer window)
    - - L-stick click - Toggle doc/local singers menu

    Effects ([Action])
    - Tap
    - - if on image - Open Image
    - - if on effect - Remove effect
    - - else - Add last chosen effect
    - Hold 
    - - D-pad Up/Down - Set/Adjust line tag for marked line
    - - D-pad Left/Right - Cycle word case
    - - [Decline] - Toggle <no effect>
    - - L-stick spin - Cycle singers/images (release to use)
    - - L-stick click - Toggle singers/images menu
    
    Edit (ZL)
    - D-pad Up/Down - Move line
    - D-pad Left/Right - Merge line up/down
    - [Confirm] - Duplicate line
    - [Decline] - Insert linebreak
    - [L-stick click] - RemoveLine
    
    Char (ZR)
    - D-pad = arrows
    - [Confirm] - Split/Join
    */

    Layer layer = g_layerLastFrame;
    if(!Gamepad::GetButton(Gamepad::Square) && !Gamepad::GetButton(Gamepad::Triangle) && !Gamepad::GetButton(Gamepad::L2) && !Gamepad::GetButton(Gamepad::R2))
    {
        layer = Standard;
    }
    if(Gamepad::GetButton(Gamepad::Square) && !Gamepad::GetButton(Gamepad::Triangle) && !Gamepad::GetButton(Gamepad::L2) && !Gamepad::GetButton(Gamepad::R2))
    {
        layer = Settings;
    }
    if(!Gamepad::GetButton(Gamepad::Square) && Gamepad::GetButton(Gamepad::Triangle) && !Gamepad::GetButton(Gamepad::L2) && !Gamepad::GetButton(Gamepad::R2))
    {
        layer = Effects;
    }
    if(!Gamepad::GetButton(Gamepad::Square) && !Gamepad::GetButton(Gamepad::Triangle) && Gamepad::GetButton(Gamepad::L2) && !Gamepad::GetButton(Gamepad::R2))
    {
        layer = Layout;
    }
    if(!Gamepad::GetButton(Gamepad::Square) && !Gamepad::GetButton(Gamepad::Triangle) && !Gamepad::GetButton(Gamepad::L2) && Gamepad::GetButton(Gamepad::R2))
    {
        layer = Adjust;
    }

    if(layer == Standard || layer == Adjust)
    {
        if(Gamepad::GetButtonDown(Gamepad::D_Up)) TimingEditor::Get().MoveMarkerUp();
        if(Gamepad::GetButtonDown(Gamepad::D_Down)) TimingEditor::Get().MoveMarkerDown();
        if(Gamepad::GetButtonDown(Gamepad::D_Left)) TimingEditor::Get().MoveMarkerLeft(layer == Adjust);
        if(Gamepad::GetButtonDown(Gamepad::D_Right)) TimingEditor::Get().MoveMarkerRight(layer == Adjust);
        if(Gamepad::GetButtonDown(Gamepad::Circle)) TimingEditor::Get().RecordEndTime();
        if(Gamepad::GetButtonDown(Gamepad::Cross))
        {
            if(layer == Standard) TimingEditor::Get().RecordStartTime();
            else TimingEditor::Get().ToggleTokenHasTime();
        }

        if(Gamepad::GetButtonDown(Gamepad::L1)) AudioPlayback::SetPlaybackSpeed(AudioPlayback::GetPlaybackSpeed() - 1);
        if(Gamepad::GetButtonDown(Gamepad::R1)) AudioPlayback::SetPlaybackSpeed(AudioPlayback::GetPlaybackSpeed() + 1);
        if(Gamepad::GetButtonDown(Gamepad::LeftStick)) AudioPlayback::Stop();
        if(Gamepad_FlickAxis(Gamepad::LeftStickX, Gamepad_FlickRIGHT))
        {
            AudioPlayback::SetPlaybackProgress(AudioPlayback::GetPlaybackProgress() + (500.f * (AudioPlayback::GetPlaybackSpeed() * .1f)));
        }
        if(Gamepad_FlickAxis(Gamepad::LeftStickX, Gamepad_FlickLEFT))
        {
            AudioPlayback::SetPlaybackProgress(AudioPlayback::GetPlaybackProgress() - (500.f * (AudioPlayback::GetPlaybackSpeed() * .1f)));
        }
        if(Gamepad_FlickAxis(Gamepad::LeftStickY, Gamepad_FlickDOWN))
        {
            if(AudioPlayback::GetIsPlaying())
            {
                AudioPlayback::Pause();
            }
            else
            {
                AudioPlayback::Play();
            }
        }
        if(Gamepad_FlickAxis(Gamepad::LeftStickY, Gamepad_FlickUP))
        {
            AudioPlayback::SetPlaybackProgress(Serialization::KaraokeDocument::Get().GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken()).myStartTime);
        }
    }
}