//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "EditMenu.h"
#include <imgui/imgui.h>
#include "Windows/TimingEditor.h"
#include "Serialization/KaraokeData.h"

void Menu::Edit_CheckShortcuts()
{
    if(TimingEditor::Get().GetInputUnsafe()) { return; }
    bool lineMode = !ImGui::IsKeyDown(ImGuiKey_ModShift) && !ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyDown(ImGuiKey_ModAlt);
    bool caseMode = ImGui::IsKeyDown(ImGuiKey_ModShift) && !ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyDown(ImGuiKey_ModAlt);
    if(lineMode && ImGui::IsKeyPressed(ImGuiKey_Enter, false))
    {
        Menu::Edit_InsertLinebreak();
    }
    if(lineMode && ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false))
    {
        Menu::Edit_MergeLineUp();
    }
    if(lineMode && ImGui::IsKeyPressed(ImGuiKey_RightArrow, false))
    {
        Menu::Edit_MergeLineDown();
    }
    if(lineMode && ImGui::IsKeyPressed(ImGuiKey_UpArrow, false))
    {
        Menu::Edit_MoveLineUp();
    }
    if(lineMode && ImGui::IsKeyPressed(ImGuiKey_DownArrow, false))
    {
        Menu::Edit_MoveLineDown();
    }
    if(lineMode && ImGui::IsKeyPressed(ImGuiKey_Space, false))
    {
        Menu::Edit_DuplicateLine();
    }
    if(lineMode && ImGui::IsKeyPressed(ImGuiKey_Backspace, false))
    {
        Menu::Edit_RemoveLine();
    }
    if(caseMode && ImGui::IsKeyPressed(ImGuiKey_UpArrow, false))
    {
        Menu::Edit_Majuscule();
    }
    if(caseMode && ImGui::IsKeyPressed(ImGuiKey_DownArrow, false))
    {
        Menu::Edit_Minuscule();
    }
    if(caseMode && ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false))
    {
        Menu::Edit_Capital();
    }
    if(caseMode && ImGui::IsKeyPressed(ImGuiKey_RightArrow, false))
    {
        Menu::Edit_ToggleCase();
    }
}

void Menu::Edit_InsertLinebreak()
{
    TimingEditor& timing = TimingEditor::Get();
    Serialization::KaraokeDocument::Get().InsertLineBreak(timing.GetMarkedLine(), timing.GetMarkedToken(), timing.GetMarkedChar());
    Serialization::KaraokeDocument::Get().MakeDirty();
}

void Menu::Edit_MergeLineUp()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    TimingEditor& timing = TimingEditor::Get();
    if(!(doc.GetLine(timing.GetMarkedLine() - 1).size() == 1 && (doc.GetToken(timing.GetMarkedLine() - 1, 0).myValue.starts_with("image") || (doc.GetLine(timing.GetMarkedLine() - 2).size() == 1 && doc.GetToken(timing.GetMarkedLine() - 2, 0).myValue.starts_with("image")))))
    {
        doc.RevoveLineBreak(timing.GetMarkedLine());
        timing.MoveMarkerUp();
    }
    doc.MakeDirty();
}

void Menu::Edit_MergeLineDown()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    TimingEditor& timing = TimingEditor::Get();
    if(!(doc.GetLine(timing.GetMarkedLine() + 1).size() == 1 && doc.GetToken(timing.GetMarkedLine() + 1, 0).myValue.starts_with("image")))
    {
        doc.RevoveLineBreak(timing.GetMarkedLine() + 1);
    }
    doc.MakeDirty();
}

void Menu::Edit_MoveLineUp()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    TimingEditor& timing = TimingEditor::Get();
    doc.MoveLineUp(timing.GetMarkedLine());
    if(timing.GetMarkedLine() > 1 && doc.GetLine(timing.GetMarkedLine() - 2).size() == 1 && doc.GetToken(timing.GetMarkedLine() - 2, 0).myValue.starts_with("image"))
    {
        doc.MoveLineUp(timing.GetMarkedLine() - 1);
    }
    timing.MoveMarkerUp();
    doc.MakeDirty();
}

void Menu::Edit_MoveLineDown()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    TimingEditor& timing = TimingEditor::Get();
    doc.MoveLineUp(timing.GetMarkedLine() + 1);
    if(doc.GetLine(timing.GetMarkedLine() + 2).size() == 1 && doc.GetLine(timing.GetMarkedLine()).size() == 1 && doc.GetToken(timing.GetMarkedLine(), 0).myValue.starts_with("image"))
    {
        doc.MoveLineUp(timing.GetMarkedLine() + 2);
    }
    timing.MoveMarkerDown();
    doc.MakeDirty();
}

void Menu::Edit_DuplicateLine()
{
    Serialization::KaraokeDocument::Get().DuplicateLine(TimingEditor::Get().GetMarkedLine());
    Serialization::KaraokeDocument::Get().MakeDirty();
}

void Menu::Edit_RemoveLine()
{
    Serialization::KaraokeDocument::Get().RemoveLine(TimingEditor::Get().GetMarkedLine());
    Serialization::KaraokeDocument::Get().MakeDirty();
}

void Edit_WordCase(bool aToUpper, bool anInvertInitial = false)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    int markedLine = TimingEditor::Get().GetMarkedLine();
    // Find first token of word
    int leadingSpaceInd = TimingEditor::Get().GetMarkedToken() - 1;
    while(leadingSpaceInd >= 0 && !doc.GetToken(markedLine, leadingSpaceInd).myValue.contains(' '))
    {
        leadingSpaceInd--;
    }
    // Find last token of word
    int trailingSpaceInd = TimingEditor::Get().GetMarkedToken();
    while(trailingSpaceInd < doc.GetLine(markedLine).size() && !doc.GetToken(markedLine, trailingSpaceInd).myValue.contains(' '))
    {
        trailingSpaceInd++;
    }
    // Replace letters
    aToUpper = anInvertInitial ? !aToUpper : aToUpper;
    if(leadingSpaceInd >= 0) for(size_t i = doc.GetToken(markedLine, leadingSpaceInd).myValue.find_last_of(' '); i < doc.GetToken(markedLine, leadingSpaceInd).myValue.size(); i++)
    {
        doc.GetToken(markedLine, leadingSpaceInd).myValue[i] = (aToUpper ? toupper : tolower)(doc.GetToken(markedLine, leadingSpaceInd).myValue[i]);
        if(anInvertInitial && isalnum(doc.GetToken(markedLine, leadingSpaceInd).myValue[i]))
        {
            anInvertInitial = false;
            aToUpper = !aToUpper;
        }
    }
    for(int i = leadingSpaceInd + 1; i < trailingSpaceInd; i++)
    {
        for(int j = 0; j < doc.GetToken(markedLine, i).myValue.size(); j++)
        {
            doc.GetToken(markedLine, i).myValue[j] = (aToUpper ? toupper : tolower)(doc.GetToken(markedLine, i).myValue[j]);
            if(anInvertInitial && isalnum(doc.GetToken(markedLine, i).myValue[j]))
            {
                anInvertInitial = false;
                aToUpper = !aToUpper;
            }
        }
    }
    for(int i = 0; i < doc.GetToken(markedLine, trailingSpaceInd).myValue.size(); i++)
    {
        if(doc.GetToken(markedLine, trailingSpaceInd).myValue[i] == ' ') { break; }
        doc.GetToken(markedLine, trailingSpaceInd).myValue[i] = (aToUpper ? toupper : tolower)(doc.GetToken(markedLine, trailingSpaceInd).myValue[i]);
        if(anInvertInitial && isalnum(doc.GetToken(markedLine, trailingSpaceInd).myValue[i]))
        {
            anInvertInitial = false;
            aToUpper = !aToUpper;
        }
    }
    doc.MakeDirty();
}

void Menu::Edit_Majuscule()
{
    Edit_WordCase(true);
}

void Menu::Edit_Minuscule()
{
    Edit_WordCase(false);
}

void Menu::Edit_Capital()
{
    Edit_WordCase(false, true);
}

void Menu::Edit_ToggleCase()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    char markedChar = doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken()).myValue[TimingEditor::Get().GetMarkedChar()];
    doc.GetToken(TimingEditor::Get().GetMarkedLine(), TimingEditor::Get().GetMarkedToken()).myValue[TimingEditor::Get().GetMarkedChar()] =
        std::isupper(markedChar) ? std::tolower(markedChar) : std::toupper(markedChar);
    doc.MakeDirty();
}
