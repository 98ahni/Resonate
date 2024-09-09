//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include <imgui.h>
#include <string>

#define IM_COL32_GET_R(col) ((ImU8*)&col)[0]
#define IM_COL32_GET_G(col) ((ImU8*)&col)[1]
#define IM_COL32_GET_B(col) ((ImU8*)&col)[2]
#define IM_COL32_GET_A(col) ((ImU8*)&col)[3]
#define IM_COL32_FROM_DOC(col) IM_COL32(IM_COL32_GET_B(col), IM_COL32_GET_G(col), IM_COL32_GET_R(col), ~(IM_COL32_GET_A(col)))
#define IM_LEFT_LABEL(widget, label, ...) ::ImGui::Text(label); ImGui::SameLine(); widget("##" label, __VA_ARGS__)

typedef unsigned int uint;
struct ImExtTexture
{
    ImTextureID myID;
    ImTextureID myHandle;
};
namespace ImGui{
    namespace Ext{
        void CreateHTMLButton(const char* anID, const char* anEvent, const char* aJSFunctonName);
        void CreateHTMLButton(ImVec2 aPosition, ImVec2 aSize, const char* anID, const char* anEvent, const char* aJSFunctonName);
        void CreateHTMLInput(const char* anID, const char* aType, const char* anEvent, const char* aJSFunctonName);
        void CreateHTMLInput(ImVec2 aPosition, ImVec2 aSize, const char* anID, const char* aType, const char* anEvent, const char* aJSFunctonName);
        void DestroyHTMLElement(const char* anID, int aDelayMillis = 0);
        void AddWindowEvent(const char* anEvent, const char* aJSFunctionName);
        void RemoveWindowEvent(const char* anEvent, const char* aJSFunctionName);

        void LoadVideo(const char* anID, const char* anFSPath);
        void PlayVideo(const char* anID);
        void PauseVideo(const char* anID);
        void SetVideoProgress(const char* anID, uint aProgress);
        void SetVideoSpeed(const char* anID, int aSpeed);
        void LoadImage(const char* anID, const char* anFSPath);
        bool RenderTexture(const char* anID, ImExtTexture& aTexture);

        void SetShortcutEvents();

        bool TimedSyllable(std::string aValue, uint aStartTime, uint anEndTime, uint aCurrentTime, bool aShowProgress, bool aUseAlpha = false);
        void SetColor(unsigned int aCol);
        void ClearColor();

        bool ToggleSwitch(const char* aLabel, bool* aValue);
        bool StepInt(const char *aLabel, int& aValue, int aSmallStep, int aLargeStep);
        bool TabMenu(ImVector<std::string> someLabels, int* aValue);
    }
}