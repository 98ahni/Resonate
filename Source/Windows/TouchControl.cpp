#include "TouchControl.h"
#include "AudioPlayback.h"
#include "TimingEditor.h"

void TouchControl::OnImGuiDraw()
{
    if(myEditor == nullptr)
    {
        myEditor = (TimingEditor*)WindowManager::GetWindow("Timing");
    }
    Gui_Begin();
    if(ImGui::Checkbox("Edit Mode", &myIsCharMode))
    {
    }
    if(ImGui::Button("Back 5s"))
    {
        AudioPlayback::SetPlaybackProgress(AudioPlayback::GetPlaybackProgress() - 500);
    }
    if(ImGui::Button("Skip 5s"))
    {
        AudioPlayback::SetPlaybackProgress(AudioPlayback::GetPlaybackProgress() + 500);
    }
    if(ImGui::Button("Up"))
    {
        myEditor->MoveMarkerUp();
    }
    if(ImGui::Button("Down"))
    {
        myEditor->MoveMarkerDown();
    }
    if(ImGui::Button("Left"))
    {
        myEditor->MoveMarkerLeft(myIsCharMode);
    }
    if(ImGui::Button("Right"))
    {
        myEditor->MoveMarkerRight(myIsCharMode);
    }
    if(ImGui::Button("Add/Remove"))
    {
        myEditor->ToggleTokenHasTime();
    }
    if(ImGui::Button("Time"))
    {
        myEditor->RecordStartTime();
    }
    if(ImGui::Button("End"))
    {
        myEditor->RecordEndTime();
    }
    Gui_End();
}