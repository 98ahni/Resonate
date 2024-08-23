#include "Preview.h"
#include <Extensions/imguiExt.h>

PreviewWindow::PreviewWindow()
{
    ImGui::Ext::LoadImage("##testImage", "ResonateIconLarger.png");
    myTexture = 0;
}

void PreviewWindow::OnImGuiDraw()
{
    Gui_Begin();
    ImTextureID tex = ImGui::Ext::RenderImage("##testImage", myTexture);
    if(tex != 0)
    {
        ImGui::Image(tex, ImGui::GetWindowSize());
    }
    Gui_End();
}