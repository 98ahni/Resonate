#include "RawText.h"
#include <misc/cpp/imgui_stdlib.h>
#include <Serialization/KaraokeData.h>
#include <Extensions/TouchInput.h>
#include "Base/WindowManager.h"
#include "TimingEditor.h"

TextEditor::TextEditor()
{
    myHasTakenFocus = false;
    myRawText = "";
}

void TextEditor::OnImGuiDraw()
{
    if(ImGui::Begin(GetName().c_str()))
    {
        if(ImGui::InputTextMultiline("##RawText", &myRawText, ImGui::GetContentRegionAvail()) && myHasTakenFocus)
        {
            Serialization::KaraokeDocument::Get().Parse(myRawText);
        }
        TouchInput_ReadyKeyboard();

        if(ImGui::IsItemHovered() || ImGui::IsWindowFocused())
        {
            if(!myHasTakenFocus)
            {
                myRawText = Serialization::KaraokeDocument::Get().Serialize();
                ((TimingEditor*)WindowManager::GetWindow("Timing"))->SetInputUnsafe(true);
            }
            myHasTakenFocus = true;
        }
        else
        {
            if(myHasTakenFocus)
            {
                ((TimingEditor*)WindowManager::GetWindow("Timing"))->SetInputUnsafe(false);
            }
            myHasTakenFocus = false;
        }
    }
    Gui_End();
}