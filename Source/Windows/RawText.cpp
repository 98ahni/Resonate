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
    if(ImGui::Begin(GetName().c_str(), 0, ImGuiWindowFlags_HorizontalScrollbar | (Serialization::KaraokeDocument::Get().GetIsDirty() ? ImGuiWindowFlags_UnsavedDocument : 0)))
    {
        if(ImGui::InputTextMultiline("##RawText", &myRawText, ImGui::GetContentRegionAvail()) && myHasTakenFocus)
        {
            Serialization::KaraokeDocument::Get().Parse(myRawText);
            Serialization::KaraokeDocument::Get().MakeDirty();
        }
        TouchInput_ReadyKeyboard();

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
    Gui_End();
}