//  This file is licenced under the GNU General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> (Used with permission from EODynamics AB) Original file author

#pragma once

void TouchInput_Init();
void TouchInput_RunInput();
// Not currently working
void TouchInput_ReadyKeyboard(bool isNum = false);
void TouchInput_CheckKeyboard();
void TouchInput_DrawKeyboardWindow();
bool TouchInput_HasTouch();