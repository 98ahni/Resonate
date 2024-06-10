#include "Properties.h"
#include <Serialization/KaraokeData.h>
#include <Extensions/imguiExt.h>

void PropertiesWindow::OnImGuiDraw()
{
    Gui_Begin();
    // Two tabs; Document and Local
    // "Document" contains the Echo headers and the singers used in the document.
    // "Local" contains a list of singers saved to the /.Resonate file.
    if(ImGui::Button("Document"))
    {
        myCurrentTab = DocumentTab;
    }
    ImGui::SameLine();
    if(ImGui::Button("Local"))
    {
        myCurrentTab = LocalTab;
    }
    switch (myCurrentTab)
    {
    case DocumentTab:
        // Font size
        // Start/end color
        // Use direct
        Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
        ImGui::SeparatorText("Color");
        ImVec4 startCol = ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(doc.myBaseStartColor));
        if(ImGui::ColorEdit4("##StartCol", &startCol.x))
        {
            doc.myBaseStartColor = ImGui::ColorConvertFloat4ToU32(startCol);
            doc.myBaseStartColor = IM_COL32_FROM_DOC(doc.myBaseStartColor);
        }
        ImVec4 endCol = ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(doc.myBaseEndColor));
        if(ImGui::ColorEdit4("##StartCol", &endCol.x))
        {
            doc.myBaseEndColor = ImGui::ColorConvertFloat4ToU32(endCol);
            doc.myBaseEndColor = IM_COL32_FROM_DOC(doc.myBaseEndColor);
        }
        ImGui::SeparatorText("Text");
        ImGui::Text("Font Size"); ImGui::SameLine(); ImGui::DragInt("##FontSize", (int*)&doc.myFontSize);
        ImGui::TextDisabled("ECHO will show %i lines.", doc.myFontSize <= 43 ? 7 : doc.myFontSize <= 50 ? 6 : 5);
        ImGui::Ext::ToggleSwitch("Use Direct Text", nullptr);
        break;
    case LocalTab:
        /* code */
        break;
    default:
        break;
    }
    Gui_End();
}

void PropertiesWindow::DrawEffectWidget(std::string anEffectAlias)
{
    // Name, Value, [Preview], EditBtn, [SaveBtn], DeleteBtn
    ImGui::BeginChild(anEffectAlias.data());
    ImVec2 size = ImGui::GetWindowSize();
    //ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_FROM_DOC(startCol));
    //ImGui::Text("<%s>", anEffectAlias.data());
    //ImGui::PopStyleColor();
    ImGui::EndChild();
}
