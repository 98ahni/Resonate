//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include "News.h"
#include "MainWindow.h"
#include <Serialization/Preferences.h>
#include <Defines.h>

#define Title(size, label) drawList->AddText(MainWindow::Font, size, ImGui::GetCursorScreenPos(), ImGui::GetColorU32(ImGuiCol_Text), label);ImGui::Dummy(MainWindow::Font->CalcTextSizeA(size, 100, 200, label))
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
    Title(15, "ver 0.9.2-beta");
    Title(40, "Autumn Cleaning");
    Title(30, "\nHere's what changed");
    BulletWrap("You can once again use copy and paste in the Raw view. ");
    BulletWrap("Images are no longer duplicated when added from the Edit menu. ");
    BulletWrap("The Preview window now shows the correct background and animations work again. ");
    BulletWrap("Resonate will no longer allow attempts to save files to the wrong cloud storage. ");
    BulletWrap("Please use the improved 'Report a bug' option in the View menu to report any unexpected behavior. ");
    ImGui::Separator();
    Title(15, "ver 0.9.1-beta");
    Title(40, "Cosmetic Update!");
    Title(30, "\nNew version of Dear ImGui");
    ImGui::TextWrapped("For those that have never heard of this /\"Dear ImGui\"/ thing, it's a code library that Resonate uses to handle most of the graphical and layout work. "
        "This upadte fixed some bugs and reworked how fonts are loaded. ");
    Title(30, "\nWhat does this mean?");
    ImGui::TextWrapped("Currently this is mostly noticable in that text is now sharper and some input errors being fixed. But this also allows for more features in the future. "
        "Another change is that the base layout got a new look and feel to be more unified. This new look also let the Raw Text view back out of the View menu since it asked so nicely. "
        "\n\nIf you notice any new bugs, please report them in the View menu. \n");
    ImGui::Separator();
    Title(15, "ver 0.9.0-beta");
    Title(40, "Resonate is More Responsive and Responsible!");
    Title(30, "\nLet's start with reliving some stress");
    ImGui::TextWrapped("Oh,no! I meant /releaving/. How ever will I undo this mistake? If only there was a shortcut, gesture or even a menu option for this.\n"
        "What's that? There is?! That's right, you can now take the stress of making a mistake off your shoulders. Ctrl/Cmd + Z/Y now undoes or redoes your last action and, don't fret, "
        "if you're not using a keyboard any system event (such as three-finger swipes on iOS) works as well. And if all else fails, they're under the Edit menu.");
    Title(30, "\nWait, are these lines out of order?");
    ImGui::TextWrapped("Oh, this line on the left seems to confirm my suspicions. And clicking on it gives shows even more info. "
        "You can even tell it to find things that might be wrong. Like an audio file that is too quiet or that too many lines could be shown simultaneously in ECHO. "
        "Whenever the Console wants to tell you something it'll even put a dot next to the View menu where it lives.");
    Title(30, "\nWell that's nice and all");
    Title(30, "     But something still seems off with the timing");
    ImGui::TextWrapped("Luckily, I just found out that Resonate has a new setting called /Enhanced Timing Readability/. Let's turn it on and see if it makes a difference. ");
    Title(22, "Woah! "); ImGui::SameLine(); ImGui::Text("Yeah, I timed that poorly...");
    ImGui::TextWrapped("Maybe my latency is wrong. oOo, I have a new preview in the Latency settings and TWO offsets, one for the visual and one for input. So I wasn't imagining things...");
    Title(30, "\nWhat is this speed?");
    ImGui::TextWrapped("The speed slider for the audio, it's done in a split second! Is that the new audio engine that uses the GPU? It's like 10 times faster! \n"
        "And the Raw view? It's so much faster!");
    Title(30, "\nI wish I could read this text over and over");
    ImGui::TextWrapped("As luck would have it, I can. It's in the View menu, next to the Help window. Handy for checking the new features at your own pace!");
    Title(30, "\n\nMore improvements?\n      There's MORE?!");
    BulletWrap("Locally saved effects are now saved correctly.");
    BulletWrap("Images should no longer disappear when setting certain values.");
    BulletWrap("Images can now be timed like regular syllables.");
    BulletWrap("The default image in preview would not always show, it now does.");
    BulletWrap("The buttons on the <line#> tag now work properly.");
    BulletWrap("Stability and visual improvements.");
    Gui_End();
}