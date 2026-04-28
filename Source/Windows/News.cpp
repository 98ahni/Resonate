//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include "News.h"
#include "MainWindow.h"
#include <Serialization/Preferences.h>
#include <Extensions/imguiExt.h>
#include <Defines.h>

#define Title(size, label) drawList->AddText(MainWindow::Font, size, ImGui::GetCursorScreenPos(), ImGui::GetColorU32(ImGuiCol_Text), label);ImGui::Dummy(MainWindow::Font->CalcTextSizeA(size, 100, 200, label))
#define GradTitle(size, label, colU, colL) ImGui::Ext::GradientText(label, ImGui::GetCursorScreenPos(), size, colU, colU, colL, colL);ImGui::Dummy(MainWindow::Font->CalcTextSizeA(size, 100, 200, label))
#define BulletWrap(text) ImGui::SetCursorPosX(ImGui::GetCursorPosX() - DPI_SCALED(10)); ImGui::Bullet(); ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + DPI_SCALED(7)); ImGui::TextWrapped(text)

NewsWindow::NewsWindow()
{
    Serialization::Preferences::SetInt("News/Version", RELEASE_VERSION);
}

void NewsWindow::OnImGuiDraw()
{
    ImGui::SetNextWindowSize({std::min(MainWindow::SwapWidth * .85f, DPI_SCALED(600.f)), std::min(MainWindow::SwapHeight * .8f, DPI_SCALED(500.f))}, ImGuiCond_Once);
    Gui_Begin();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    Title(20, "ver 0.10.0-prerelease");
    Title(50, "Resonate is now in Pre-Release!");
    Title(40, "\nWait really!?");
    ImGui::TextWrapped("Yes, really. After two years of talking non-stop about this app it's finally content complete. That means all the features I've planned are done. "
        "Now, that doesn't mean there won't be bugs. So, if you see any unexpected behavior, please report them through the option /Report a Bug/ in the /View/ menu. ");
    Title(40, "\nSo, what's actually new?");
    ImGui::TextWrapped("First of all: this new font! As well as new fallbacks to allow for pīnyīn and other fun symbols☆");
    GradTitle(30, "There is now also support for GRADIENTS!", IM_COL32(0x38, 0xF9, 0x7C, 255), IM_COL32(0xFF, 0xCC, 0xE9, 255));
    ImGui::TextWrapped("This is to fit two voices singing in unison on the same line. Any color effect added in the Properties can now be applied as a gradient as well. "
        "This makes it easier to fit everything when there are a lot of singers or the song gets really fast.");
    ImGui::TextWrapped("Something else that helps with fast songs is direct text. Which can now be used on a per line basis! In Properties, the toggle for direct text has been "
        "swapped out for what to default to. The new tags <direct> and <cascade> can then be used to invert the behavior of that default. It's found in the Effects menu. ");
    ImGui::TextWrapped("But wouldn't it be great if this somehow affected the Preview window as well? And maybe even give a warning if <direct> might be nessecary?\n"
        "Well, isn't it lucky that the Preview window does exactly that! The text now cascades in to represent the final product better.");
    Title(40, "\nSpeaking of the Preview window...");
    ImGui::TextWrapped("Besides the text animation and gradients; it's also much more reliable at placing lines. That goes double for lines using <line#> as they are "
        "now propperly respected by other lines. With this also comes more reliable warnings in the Console. Even new ones aimed at finding conflicts when there is too "
        "much text trying to fit on the screen. ");
    Title(40, "\nEven the Timing View gets some love");
    ImGui::TextWrapped("Does a line need re-timing after all of the effects have been added? Don't worry. The Timing View now looks for adjacent tags and applies "
        "the time to those as well. The text marker also moves past tags being timed to time the syllable it was meant for. It's like the tags are not even there. ");
    Title(40, "\n\nLots of other changes");
    BulletWrap("Images in the Preview window can now be different sizes.");
    BulletWrap("The Preview window is no longer an epilepsy risk when scrubbing.");
    BulletWrap("Improved time precision of the VexWarp engine.");
    BulletWrap("Improved the performance of the RubberBand engine.");
    BulletWrap("When syllabifying a single line, a time stamp is now added to the beginning as well.");
    BulletWrap("The enter/return key can now be used safely to confirm inputs.");
    BulletWrap("Effects can now be transferred from a document to local storage.");
    BulletWrap("Images can now be moved using the Edit menu or Alt+arrows.");
    BulletWrap("Hardware latency is now accounted for behind the scenes.");
    BulletWrap("When importing from Google Drive, special characters are now read correctly.");
    BulletWrap("It is now more difficult to corrupt effects in Raw View.");
    Gui_End();
}