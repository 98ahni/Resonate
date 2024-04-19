#include "Todo.h"

#define TODO(item) ImGui::BulletText(item)

void TodoWindow::OnImGuiDraw()
{
    Gui_Begin();

    TODO("Implement this window.");
    TODO("Figure out why the TimingEditor doesn't take input until RawText has been viewed.");
    TODO("Figure out why the RawText doesn't work until it's been deselected after being selected.");
    TODO("Figure out why the speed slider has to be touched.");
    TODO("Figure out why the create audio function gets called twice.");

    Gui_End();
}