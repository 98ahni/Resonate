//  This file is licenced under the GNU General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Help.h"
#include "MainWindow.h"

HelpWindow::HelpWindow()
{

}

#define StartTreeNode(label) if(ImGui::TreeNode(label)){ImGui::PopFont()
#define EndTreeNode ImGui::TreePop();ImGui::PushFont(MainWindow::Font);}
void HelpWindow::OnImGuiDraw()
{
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
    ImGui::TextWrapped("The Audio window contains a Play and Pause button, the playback progress and a speed slider. The playback can be controlled by system or headphone controls as well.");
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
    "After the built in effects are the effects used in the document then the ones saved to browser storage. "
    "You can create effects by selecting Document Properties in the Edit menu. Here you can add or remove effects and store them to the browser storage. "
    "A named effect will appear as its name in the Timing editor but when exporting will be translated to a command readable by ECHO.");
    EndTreeNode

    StartTreeNode("Settings");
    ImGui::TextWrapped("In View > Settings you can set an offset to counter headphone delay, save and load internal data or restore the app to default.");
    EndTreeNode

    StartTreeNode("Dealing with Raw Text");
    ImGui::TextWrapped("The Raw Text view is meant to make changes to the lyrics before timing");
    EndTreeNode

    StartTreeNode("Additional Tricks");
    ImGui::TextWrapped("");
    EndTreeNode

    StartTreeNode("Touch Controls");
    ImGui::TextWrapped("");
    EndTreeNode

    StartTreeNode("Keyboard Controls");
    ImGui::TextWrapped("");
    EndTreeNode

    ImGui::PopFont();
    Gui_End();
}