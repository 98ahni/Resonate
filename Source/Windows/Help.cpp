//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Help.h"
#include "MainWindow.h"
#include <Defines.h>

HelpWindow::HelpWindow()
{

}

// When adding a font for keybinds, look at 0x2408 onwards and 0x2460

#define StartTreeNode(label) if(ImGui::TreeNode(label)){ImGui::PopFont()
#define EndTreeNode ImGui::TreePop();ImGui::PushFont(MainWindow::Font);}
#define BulletWrap(text) ImGui::SetCursorPosX(ImGui::GetCursorPosX() - DPI_SCALED(10)); ImGui::Bullet(); ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + DPI_SCALED(7)); ImGui::TextWrapped(text)
#define Keybind(key, text) ImGui::Text(key); ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - DPI_SCALED(17)); ImGui::Bullet(); ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetCursorPosX() + DPI_SCALED(7)); ImGui::TextWrapped(text)
void HelpWindow::OnImGuiDraw()
{
    ImGui::SetNextWindowSize({std::min(MainWindow::SwapWidth * .85f, DPI_SCALED(600.f)), std::min(MainWindow::SwapHeight * .8f, DPI_SCALED(500.f))}, ImGuiCond_Once);
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
    ImGui::TextWrapped("When opening a project you can choose to open it from the local machine, Google Drive or Dropbox. "
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
    "When listening to check the syllable split a speed of 60%% is recomended, when timing use 40%% and when checking your finished result it's good to use 80%%. \n"
    "When first starting the app, you need to click/tap anywhere in the app to start the audio processor. "
    "After that when first selecting a speed the sound will play after a few seconds and then switch to a higher quality after up to a minute. ");
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
    ImGui::TextWrapped("In the Edit menu you can split lines into two where the text marker is, merge lines with their neighbors, duplicate them and switch their places. "
    "You can also change casing, either per word to quickly change a lot of text or per letter to get into the details. ");
    EndTreeNode

    StartTreeNode("Document Properties");
    ImGui::TextWrapped("In the Document Properties you can set effects that affect the whole document and add or remove effects that can be stored to the browser storage. \n"
    "The Document tab contains options stored in the document itself while the Local tab has effects stored locally and can be used in any project. \n "
    "The start and end colors defined here are the default colors used in the document. \n "
    "Font Size affects how many lines can be shown at a time in ECHO, up to seven. \n "
    "Using Direct Text is used when the singer is singing faster than ECHO animates the text which rarely happens. \n "
    "Shift Timings will shift all time codes in the document by the specified amount. "
    );
    EndTreeNode

    StartTreeNode("Adding Effects");
    ImGui::TextWrapped("Adding effects is as simple as clicking the Effects menu and selecting the appropriate one. ");
    ImGui::PushFont(MainWindow::Font);
    StartTreeNode("Setting the Line");
    ImGui::TextWrapped("By selecting Display Line a <line#> tag will be added to the selected line in the Timing editor. "
    "The value can then be changed in the Timing editor any time. \nTo remove it, move the text marker to the line and un-select the Effects > Display Line option");
    EndTreeNode
    StartTreeNode("No Effect");
    ImGui::TextWrapped("This option will remove all text animation in ECHO and make the text white. It is most often used when someone is talking in the song instead of singing. \n"
    "To remove it, move the text marker to the line and un-select the Effects > No Effect option");
    EndTreeNode
    StartTreeNode("Text Color");
    ImGui::TextWrapped("After the built in line effects are the text effects added to the document. "
    "You can create effects in the Document Properties. "
    "A named effect will appear as its name in the Timing editor but when exporting will be translated to a command readable by ECHO.");
    EndTreeNode
    StartTreeNode("Images");
    ImGui::TextWrapped("This dropdown shows all images in the project. Selecting one will add an image tag to the Timing editor, visible as a small thumbnail. "
    "An image has several options that can be changed by selecting it in the Timing editor and then selecting Apply when done. \n"
    "When adding an image or applying options it is sorted according to its time of appearance. However, it does not move when its time is changed in other ways such as Raw Text. "
    "For information on why sorting is important, please read the guidelines (View > Guidelines). \n"
    "Images can not be used if a video is present in the project!");
    EndTreeNode
    ImGui::PopFont();
    EndTreeNode

    StartTreeNode("Previewing");
    ImGui::TextWrapped("In the Effects menu is the option to play a preview of your work. "
    "The playback is controlled by the Audio window so that it stays in sync with any changes made. "
    "This view is meant to check composition and sizing and will always play in the correct aspect ratio of 16:9. "
    "(At this stage it's far from accurate to how ECHO will play the file.)"
    "\nSupported file formats are *.jpg, *.png and *.mp4.");
    EndTreeNode

    StartTreeNode("Settings");
    ImGui::TextWrapped("In View > Settings you can set an offset to counter headphone delay, choose audio processor to balance speed to quality and change rendering settings. "
    "You can also save and load internal data to transfer your setup to another device or restore the app to default.");
    EndTreeNode

    StartTreeNode("Dealing with Raw Text");
    ImGui::TextWrapped("The Raw Text view is meant to make changes to the lyrics before timing in case you don't have access to an extrnal text editor. "
    "It can be accessed by selecting the Raw Text tab next to the Timing tab right below the menu bar at the top of the screen. "
    "It shows and allows you to edit the file that is readable by both ECHO and Resonate. Please note that if an effect value is changed it might not be recognized as a Resonate effect. "
    "Changing the Resonate headers are not recommended as it may result in undefined behavior.");
    EndTreeNode
    
    StartTreeNode("Installing as a PWA");
    ImGui::TextWrapped("Resonate does not require any installation process. However, it can be installed as a Progressive Web App (PWA) for convenience. "
    "A PWA looks like a normal app on your device, without most clutter of the web browser. The process vary by browser as listed below. \n");
    Keybind("Chrome       ", "On all chromium based browsres the install button appears in the address bar on the right side. Click that button and then confirm the install. "
    "The app will open and a shortcut is added to the desktop. To uninstall, click the three dots in the upper right of the PWA and select 'Uninstall'.");
    Keybind("Safari, iOS  ", "Tap the [↑] Share icon then select \"[+] Add to Homescreen\" in the menu. Confirm the install and the PWA will appear on your homescreen. "
    "To uninstall, remove it like a normal app. ");
    Keybind("Safari, macOS", "Click the [↑] Share icon or open Safari's File menu, then select \"Add to Dock\" in the menu. Confirm the install and the PWA will appear in your dock. "
    "To uninstall, remove it like a normal app. ");
    Keybind("Firefox      ", "You need to install an extension to use PWAs in Firefox. Install method varies depending on which one you use. ");
    Keybind("Android      ", "Tap the three dots in the upper right corner of the screen, then select \"[+ Install app\". Confirm the install and the PWA will appear on your homescreen. "
    "To uninstall, remove it like a normal app. ");
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
    Keybind("[Space]           ", "Set timing at text marker and advance. ");
    Keybind("[Enter]           ", "Insert or set a pause to time at text marker. ");
    Keybind("Arrow keys        ", "Move text marker in specified direction one syllable at a time. ");
    Keybind("[Ctrl]+[Space]    ", "Add or remove a syllable split. ");
    Keybind("[Ctrl]+Arrows     ", "Move text marker in specified direction one character at a time. ");
    Keybind("[Shift]+Click     ", "Set Audio Playback progress to the time of the syllable clicked. ");
    Keybind("[Shift]+[Space]   ", "Play/Pause the Audio Playback. ");
    Keybind("[Shift]+[Enter]   ", "Stop the Audio Playback. ");
    Keybind("[Shift]+Up/Down   ", "Increse/Decrese the speed of the Audio Playback. ");
    Keybind("[Shift]+Left/Right", "Rewind/Fast forward the Audio Playback by five seconds of listening. ");
    Keybind("[Alt]+[Space]     ", "Duplicate the current line. ");
    Keybind("[Alt]+[Enter]     ", "Insert a linebreak at the text marker. ");
    Keybind("[Alt]+Up/Down     ", "Move line up/down. ");
    Keybind("[Alt]+Left/Right  ", "Merge line up/down. ");
    Keybind("[Alt]+[Backspace] ", "Remove line. ");
    Keybind("[Alt]+[Shift]+Up  ", "Format word as MAJUSCULE. ");
    Keybind("[Alt]+[Shift]+Down", "Format word as minuscule. ");
    Keybind("[Alt]+[Shift]+Left", "Format word as Capital. ");
    Keybind("[Alt]+[Shft]+Right", "Toggel case of current character. ");
    EndTreeNode

    StartTreeNode("Gamepad Controls");
    ImGui::TextWrapped("Most of Resonate's functionality can be reached from a gamepad. "
    "In order to make as much as possible available there are four layers which are as follows:");
    Keybind("PS:(/\\) X/N:(Y)       ", "Effects. Tapping lets you do quick actions. Holding accesses the effect control layer. ");
    Keybind("    Quick actions     ", "Opens an image, removes an effect tag or inserts the last one used. ");
    Keybind("PS:([]) X/N:(X)       ", "Settings. Tapping toggles the overlay. Holding accesses the setting control layer. ");
    Keybind("PS:(L2) X:(LT) N:(ZL) ", "Layout. Holding lets you edit lines like holding [Alt]. ");
    Keybind("PS:(R2) X:(RT) N:(ZR) ", "Adjust. Holding lets you move the text marker by the character and change syllabification. ");
    EndTreeNode

    ImGui::PopFont();
    Gui_End();
}