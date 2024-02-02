

#define IM_COL32_GET_R(col) ((ImU8*)&col)[0]
#define IM_COL32_GET_G(col) ((ImU8*)&col)[1]
#define IM_COL32_GET_B(col) ((ImU8*)&col)[2]
#define IM_COL32_GET_A(col) ((ImU8*)&col)[3]
namespace Serialization {struct KaraokeToken;}
namespace ImGui{
    namespace Ext{
        bool TimedSyllable(Serialization::KaraokeToken aSyllable, float aCurrentTime);
        void SetColor(unsigned int aCol);
        void ClearColor();
    }
}