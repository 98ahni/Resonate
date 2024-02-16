#include "RawText.h"
#include <misc/cpp/imgui_stdlib.h>
#include <Serialization/KaraokeData.h>

void TextEditor::OnImGuiDraw()
{
    if(Gui_Begin())
    {
        if(ImGui::IsWindowFocused())
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

        if(ImGui::InputTextMultiline("##RawText", &myRawText, ImGui::GetContentRegionAvail()))
        {
            Serialization::KaraokeDocument::Get().Parse(myRawText);
        }
    }
    Gui_End();
}