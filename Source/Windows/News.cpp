//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include "News.h"
#include "MainWindow.h"
#include <Serialization/Preferences.h>
#include <Defines.h>

#define Title(size, label) drawList->AddText(MainWindow::Font, size, ImGui::GetCursorPos(), ImGui::GetColorU32(ImGuiCol_Text), label);ImGui::Dummy(MainWindow::Font->CalcTextSizeA(size, 100, 200, label))
#define BulletWrap(text) ImGui::SetCursorPosX(ImGui::GetCursorPosX() - DPI_SCALED(10)); ImGui::Bullet(); ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + DPI_SCALED(7)); ImGui::TextWrapped(text)

NewsWindow::NewsWindow()
{
    Serialization::Preferences::SetInt("News/Version", RELEASE_VERSION);
}

void NewsWindow::OnImGuiDraw()
{
    Gui_Begin();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    Title(15, "ver 0.9.0-beta");
    Gui_End();
}