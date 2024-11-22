//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"
#include <unordered_map>

namespace Serialization
{
    struct KaraokeEffect;
    struct KaraokeColorEffect;
    typedef std::unordered_map<std::string, KaraokeEffect*> KaraokeAliasMap;
}
typedef unsigned int uint;
class PropertiesWindow : public EditorWindow
{
public:
    PropertiesWindow();
    void OnImGuiDraw() override;
    bool DrawFontSizeGamepadPopup();
    bool DrawShiftTimingsGamepadPopup();
    bool DrawDefaultColorsGamepadPopup(int aSelectedSlider);
    bool DrawSingerColorsGamepadPopup(int aSelectedSlider, bool anIsLocal, std::string anEditingName);
    // Shorthand for Serialization::KaraokeDocument::Get().GetEffectAliases();
    const Serialization::KaraokeAliasMap& GetDocumentEffectAliases();
    const Serialization::KaraokeAliasMap& GetLocalEffectAliases();

private:
    enum TabIndex
    {
        DocumentTab,
        LocalTab
    };

    void ShiftTimingsPopupDraw();
    void DrawEffectWidget(std::string anEffectAlias, Serialization::KaraokeEffect* anEffect);
    void ApplyEdit(Serialization::KaraokeEffect* anEffect);
    bool DrawGamepadColorComponent(const char* aLabel, bool aIsSelected, int& aColorComponent);
    bool DrawColorGamepadMenu(int aSelectedSlider, uint& aStartColor, uint& anEndColor);

    bool myShiftTimingsPopupOpen;
    int myShiftTimingsValue;
    TabIndex myCurrentTab;
    std::string myEditingEffect;
    std::string myNewEffectName;
    Serialization::KaraokeColorEffect* myNewEffectToAdd = nullptr;
    Serialization::KaraokeAliasMap myLocalEffectAliases;
};