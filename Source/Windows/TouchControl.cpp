#include "TouchControl.h"

void TouchControl::OnImGuiDraw()
{
    Gui_Begin();
    if(ImGui::Checkbox("Edit Mode", &myIsCharMode))
    {
    }
    if(ImGui::Button("Back 5s"))
    {
    }
    if(ImGui::Button("Skip 5s"))
    {
    }
    if(ImGui::Button("Up"))
    {
    }
    if(ImGui::Button("Down"))
    {
    }
    if(ImGui::Button("Left"))
    {
    }
    if(ImGui::Button("Right"))
    {
    }
    if(ImGui::Button("Add/Remove"))
    {
    }
    if(ImGui::Button("Time"))
    {
    }
    if(ImGui::Button("End"))
    {
    }
    Gui_End();
}