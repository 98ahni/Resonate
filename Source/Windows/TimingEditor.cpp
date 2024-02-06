#include "TimingEditor.h"
#include <Serialization/KaraokeData.h>
#include <Extensions/imguiExt.h>

void TimingEditor::OnImGuiDraw()
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    if(Gui_Begin())
    {
        for(int line = 0; line < doc.GetData().size(); line++)
        {
            for(int token = 0; token < doc.GetLine(line).size(); token++)
            {
                ImGui::Ext::TimedSyllable(doc.GetLine(line)[token], float());
            }
        }
    }
    Gui_End();
}