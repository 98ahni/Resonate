#include <imgui.h>
#include <string>

#define IM_COL32_GET_R(col) ((ImU8*)&col)[0]
#define IM_COL32_GET_G(col) ((ImU8*)&col)[1]
#define IM_COL32_GET_B(col) ((ImU8*)&col)[2]
#define IM_COL32_GET_A(col) ((ImU8*)&col)[3]
typedef unsigned int uint;
namespace ImGui{
    namespace Ext{
        typedef void (*HTMLEvent)();
        void CreateHTMLButton(const char* anID, const char* anEvent, const char* aJSFunctonName);
        void CreateHTMLButton(const char* anID, const char* anEvent, HTMLEvent& aCallback);
        void CreateHTMLButton(ImVec2 aPosition, ImVec2 aSize, const char* anID, const char* anEvent, const char* aJSFunctonName);
        void CreateHTMLButton(ImVec2 aPosition, ImVec2 aSize, const char* anID, const char* anEvent, HTMLEvent& aCallback);
        void CreateHTMLInput(const char* anID, const char* aType, const char* anEvent, const char* aJSFunctonName);
        void CreateHTMLInput(const char* anID, const char* aType, const char* anEvent, HTMLEvent& aCallback);
        void CreateHTMLInput(ImVec2 aPosition, ImVec2 aSize, const char* anID, const char* aType, const char* anEvent, const char* aJSFunctonName);
        void CreateHTMLInput(ImVec2 aPosition, ImVec2 aSize, const char* anID, const char* aType, const char* anEvent, HTMLEvent& aCallback);
        void DestroyHTMLElement(const char* anID, int aDelayMillis = 0);

        bool TimedSyllable(std::string aValue, uint aStartTime, uint anEndTime, uint aCurrentTime);
        void SetColor(unsigned int aCol);
        void ClearColor();
    }
}