//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include "ViewSwitcher.h"
#include "Defines.h"
#include "MainWindow.h"

ViewSwitcher::ViewSwitcher()
{
}

void ViewSwitcher::OnImGuiDraw()
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
    float wndHeight = ImGui::GetFrameHeightWithSpacing();
    if(ImGui::BeginChild("ViewSwitcher", {0, wndHeight}, ImGuiChildFlags_Border, ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {DPI_SCALED(1), 0});
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, {.5f, .35f});
        ImGui::SetCursorPos({0, DPI_SCALED(5)});
        auto itBtn = myNames.begin();
        while (itBtn != myNames.end())
        {
            if(ImGui::Button(itBtn->c_str(), {ImGui::GetWindowWidth() / myNames.size(), wndHeight}))
            {
                myCurrentView = *itBtn;
            }
            ImGui::SameLine();
            itBtn++;
        }
        ImGui::Dummy({0, 0});
        ImGui::PopStyleVar(2);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    //MainWindow::DockSizeOffset.y += ImGui::GetFrameHeightWithSpacing();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - DPI_SCALED(7));
    myWindows[myCurrentView]->OnImGuiDraw();
}

EditorWindow* ViewSwitcher::GetWindow(const std::string& aName)
{
	if (myWindows.contains(aName))
	{
		return myWindows[aName];
	}
	//LOG_WARN("Window name doesn't exist!");
	return nullptr;
}