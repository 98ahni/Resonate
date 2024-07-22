//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Help.h"
#include "MainWindow.h"

HelpWindow::HelpWindow()
{

}

#define StartTreeNode(label) if(ImGui::TreeNode(label)){ImGui::PopFont()
#define EndTreeNode ImGui::TreePop();ImGui::PushFont(MainWindow::Font);}
#define BulletWrap(text) ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10); ImGui::Bullet(); ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7); ImGui::TextWrapped(text)
#define Keybind(key, text) ImGui::Text(key); ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 17); ImGui::Bullet(); ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7); ImGui::TextWrapped(text)
void HelpWindow::OnImGuiDraw()
{
    ImGui::SetNextWindowSize({std::min(ImGui::GetContentRegionAvail().x * .85f, 500.f), ImGui::GetContentRegionAvail().y * .8f}, ImGuiCond_FirstUseEver);
    Gui_Begin();
    ImGui::PushFont(MainWindow::Font);
    ImGui::Text("Using Resonate");
    ImGui::PopFont();
    ImGui::TextWrapped("Resonate is built for use on mobile devices and, since it requires no install, any device you have at hand.");
    ImGui::TextWrapped("If you have never timed a song in Resonate before, please read the Before Timing section below. "
    "If you are already familiar with Hibikase many things in this guide will feel familiar.");
    ImGui::PushFont(MainWindow::Font);

    StartTreeNode("Before Timing");
    ImGui::TextWrapped("There are a few things you will need to do before you start timing. Namely look at the MoonCat Guidelines in About > Guidelines. "
    "These guidelines are the same no matter what software you use to time songs. There are a few things in it specific to Hibikase but they are always specified as such. "
    "Next is to find a suitable song to time. ECHO supports audio in the form of .mp3 and .flac while Resonate currently only supports .mp3 files. "
    "The lyrics should be in a .txt file and Resonate supports graphics in the formats .mp4, .png and .jpg (once the preview window is done). All files should be created before loading Resonate.");
    EndTreeNode

    StartTreeNode("Loading a Project");
    ImGui::TextWrapped("When opening a project you can choose to open it from the local machine, Google Drive or (later) Dropbox. "
    "If you open the project from a local storage on iOS or Android, you will have to open each file individually. "
    "When loading from Google Drive or Dropbox you will first have to log in to the chosen service, then select the folder containing the project. "
    "Resonate can not access any files outside of the folder you chose.");
    EndTreeNode

    StartTreeNode("Syllabify the Project");
    ImGui::TextWrapped("After opening the project you should now see the lyrics file in the Timing window. If the lyrics is not already split into syllables then click "
    "Syllabify in the top menu then go to Document or Line and finally the language the song is written in. The Line option only splits the line where the text marker is in the Timing window. "
    "If the automatic splitting made a mistake you can add or remove splits by moving the text marker to the place you like with Ctrl + Arrow keys then pressing Ctrl + Space to add/remove a split. ");
    EndTreeNode

    StartTreeNode("The Audio Player");
    ImGui::TextWrapped("The Audio window contains a Play and Pause button, the playback progress and a speed slider. "
    "When listening to check the syllable split a speed of 60%% is recomended, when timing use 40%% and when checking your finished result it's good to use 80%%. ");
    EndTreeNode

    StartTreeNode("Timing Basics");
    ImGui::TextWrapped("To set timing of a syllable press Space when the audio hits the syllable that the text marker is on. The marker will then progress to the next syllable. "
    "To add a pause between syllables, press Enter when the previous syllable should end. Adding a pause will not progress the marker. If you want to remove a pause, use Ctrl + Space.");
    EndTreeNode

    StartTreeNode("Saving the Project");
    ImGui::TextWrapped("You can only save the document to the same service you opened it from and to local storage. Resonate also uses auto-saving to make sure no data is lost "
    "in case the page reloads, the browser closes or the device turns off. Auto-saves are stored in the browsers storage and might be removed if the device is low on memory. "
    "Don't rely on them for long-term storage.");
    EndTreeNode

    StartTreeNode("Simple Text Edits");
    ImGui::TextWrapped("In the Edit menu you can split lines into two where the text marker is, merge lines with their neighbors, duplicate them and switch their places. ");
    EndTreeNode

    StartTreeNode("Adding Effects");
    ImGui::TextWrapped("Adding effects is as simple as clicking the Effects menu and selecting the appropriate one. "
    "After the built in line effects are the text effects added to the document. "
    "You can create effects by selecting Document Properties in the View menu. Here you can add or remove effects and store them to the browser storage. "
    "A named effect will appear as its name in the Timing editor but when exporting will be translated to a command readable by ECHO.");
    EndTreeNode

    StartTreeNode("Settings");
    ImGui::TextWrapped("In View > Settings you can set an offset to counter headphone delay, save and load internal data or restore the app to default.");
    EndTreeNode

    StartTreeNode("Dealing with Raw Text");
    ImGui::TextWrapped("The Raw Text view is meant to make changes to the lyrics before timing in case you don't have access to an extrnal text editor. "
    "It shows and allows you to edit the file that is readable by both ECHO and Resonate. Please note that if an effect value is changed it might not be recognized as a Resonate effect. "
    "Changing the Resonate headers are not recommended as it may result in undefined behavior.");
    EndTreeNode
    
    StartTreeNode("Installing as a PWA");
    ImGui::TextWrapped("Resonate does not require any installation process. However, it can be installed as a Progressive Web App (PWA) for convenience. "
    "A PWA looks like a normal app on your device, without most clutter of the web browser. The process vary by browser as listed below. \n");
    Keybind("Chrome       ", "On all chromium based browsres the install button appears in the address bar on the right side. Click that button and then confirm the install. "
    "The app will open and a shortcut is added to the desktop. To uninstall, click the three dots in the upper right of the PWA and select 'Uninstall'.");
    Keybind("Safari, iOS  ", "Tap the [↑] Share icon then select [+] Add to Homescreen in the menu. Confirm the install and the PWA will appear on your homescreen. "
    "To uninstall, remove it like a normal app. ");
    Keybind("Safari, macOS", "(Untested) Presumably same as one of the above.");
    Keybind("Firefox      ", "(Unknown)");
    Keybind("Android      ", "(Unknown)");
    EndTreeNode

    StartTreeNode("Additional Tricks");
    BulletWrap("If you are familiar with Hibikase's separation between Timing and Edit you can think of it as holding Ctrl for Edit mode and releasing it for Timing mode.");
    BulletWrap("If another singer joins in the middle of a line you can split it where they join, duplicate that part and join the two original parts together with the Edit menu.");
    BulletWrap("The Audio Playback can be controlled by system or headphone controls as well.");
    EndTreeNode

    StartTreeNode("Touch Controls");
    ImGui::TextWrapped("The Touch Controls window can be opened from the View menu. It has buttons for navigating the Timing Editor and Audio Playback."
    "The buttons ↻ and ↺ will fast forward and rewind the audio while the ◣ and ◢ buttons will set timing and add a pause respectively and "
    "the center button [:] will add or remove a syllable split. The arrows will move the text marker in the specified direction and, lastly, the "
    "Edit Mode toggle will switch between moving the text marker per character and per syllable. ");
    ImGui::TextWrapped("You can scroll in any window using two fingers. ");
    ImGui::TextWrapped("When using a touch screen you may need to double tap text boxes for the virtual keyboard to show.");
    EndTreeNode

    StartTreeNode("Keyboard Controls");
    Keybind("[Space]", "Set timing at text marker and advance. ");
    Keybind("+[Ctrl]", "Add or remove a syllable split. ");
    Keybind("[Enter]", "Insert or set a pause to time at text marker. ");
    Keybind("Arrow keys", "Move text marker in specified direction one syllable at a time. ");
    Keybind("+[Ctrl]", "Move text marker in specified direction one character at a time. ");
    Keybind("[Shift]+Click", "Set Audio Playback progress to the time of the syllable clicked. ");
    EndTreeNode

    ImGui::PopFont();
    Gui_End();
}