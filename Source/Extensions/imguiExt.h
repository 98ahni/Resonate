

#define IM_COL32_GET_R(col) ((ImU8*)&col)[0]
#define IM_COL32_GET_G(col) ((ImU8*)&col)[1]
#define IM_COL32_GET_B(col) ((ImU8*)&col)[2]
#define IM_COL32_GET_A(col) ((ImU8*)&col)[3]

namespace ImGui{
    namespace Ext{
        void TimedSyllable(const char* syllable, float start, float end, float currentTime);
        void SetColor(unsigned int col);
        void ClearColor();
    }
}