//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "RawText.h"
#include <emscripten.h>
#include <misc/cpp/imgui_stdlib.h>
#include <Serialization/KaraokeData.h>
#include <Extensions/TouchInput.h>
#include "Base/WindowManager.h"
#include "TimingEditor.h"

TextEditor::TextHistory::TextHistory(std::string aRawText)
{
    myType = History::Record::Edit;
    mySavedRawText = aRawText;
}
void TextEditor::TextHistory::Undo()
{
    std::string temp = Serialization::KaraokeDocument::Get().Serialize();
    Serialization::KaraokeDocument::Get().Parse(mySavedRawText);
    Serialization::KaraokeDocument::Get().MakeDirty();
    mySavedRawText = temp;
    ourShouldReload = true;
}
void TextEditor::TextHistory::Redo()
{
    Undo();
}

TextEditor::TextEditor()
{
    myHasTakenFocus = false;
    myRawText = "";
    myShouldSerialize = false;
    myLastEditTime = 0;
    ourShouldReload = false;
}

void TextEditor::OnImGuiDraw()
{
    //if(ImGui::Begin(GetName().c_str(), 0, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_HorizontalScrollbar | (Serialization::KaraokeDocument::Get().GetIsDirty() ? ImGuiWindowFlags_UnsavedDocument : 0)))
    if(Gui_Begin(ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_HorizontalScrollbar | (Serialization::KaraokeDocument::Get().GetIsDirty() ? ImGuiWindowFlags_UnsavedDocument : 0)))
    {
        std::string raw = myRawText;
        if(!ourShouldReload && ImGui::InputTextMultiline("##RawText", &myRawText, ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_NoUndoRedo) && myHasTakenFocus)
        {
            if(!myShouldSerialize) {History::AddRecord(new TextHistory(raw));}
            myLastEditTime = emscripten_get_now();
            myShouldSerialize = true;
        }
        TouchInput_ReadyKeyboard();

        if(myShouldSerialize && myLastEditTime + 1000 < emscripten_get_now())
        {
            myShouldSerialize = false;
            Serialization::KaraokeDocument::Get().Parse(myRawText);
            Serialization::KaraokeDocument::Get().MakeDirty();
        }

        if(!myHasTakenFocus || ImGui::IsWindowAppearing() || ImGui::IsItemActivated() || ourShouldReload)
        {
            myRawText = Serialization::KaraokeDocument::Get().Serialize();
            ((TimingEditor*)WindowManager::GetWindow("Timing"))->SetInputUnsafe(true);
            ((TimingEditor*)WindowManager::GetWindow("Timing"))->SetInputUnsafe(false);
            myHasTakenFocus = true;
            ourShouldReload = false;
        }
    }
    else
    {
        if(myHasTakenFocus)
        {
            if(myShouldSerialize)
            {
                Serialization::KaraokeDocument::Get().Parse(myRawText);
                Serialization::KaraokeDocument::Get().MakeDirty();
            }
            ((TimingEditor*)WindowManager::GetWindow("Timing"))->SetInputUnsafe(false);
        }
        myHasTakenFocus = false;
    }
    Gui_End();
}