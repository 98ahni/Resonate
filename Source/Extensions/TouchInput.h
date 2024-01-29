#pragma once

void TouchInput_Init();
void TouchInput_RunInput();
// Not currently working
void TouchInput_ReadyKeyboard(bool isNum = false);
void TouchInput_CheckKeyboard();
void TouchInput_DrawKeyboardWindow();
bool TouchInput_HasTouch();