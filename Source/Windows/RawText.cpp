#include "RawText.h"
#include <misc/cpp/imgui_stdlib.h>
#include <Serialization/KaraokeData.h>
#include <Extensions/TouchInput.h>

void TextEditor::OnImGuiDraw()
{
    if(ImGui::Begin(GetName().c_str()))
    {
        if(ImGui::IsWindowHovered())
        {
            if(!myHasTakenFocus)
            {
                myRawText = Serialization::KaraokeDocument::Get().Serialize();
            }
            myHasTakenFocus = true;
        }
        else
        {
            myHasTakenFocus = false;
        }

        if(ImGui::InputTextMultiline("##RawText", &myRawText, ImGui::GetContentRegionAvail()) && myHasTakenFocus)
        {
            Serialization::KaraokeDocument::Get().Parse(myRawText);
        }
        TouchInput_ReadyKeyboard();
    }
    Gui_End();
}