#include "TimingEditor.h"
#include <Serialization/KaraokeData.h>
#include <Extensions/imguiExt.h>

void TimingEditor::OnImGuiDraw()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    if(Gui_Begin())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 10});
        for(int line = 0; line < doc.GetData().size(); line++)
        {
            for(int token = 0; token < doc.GetLine(line).size(); token++)
            {
                uint start = doc.GetLine(line)[token].myHasStart ? doc.GetLine(line)[token].myStartTime : 0;
                uint end = doc.GetTokenAfter(line, token).myHasStart ? doc.GetTokenAfter(line, token).myStartTime : start;
                ImGui::Ext::TimedSyllable(doc.GetLine(line)[token].myValue, start, end, 0);
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
        ImGui::PopStyleVar();
    }
    Gui_End();
}