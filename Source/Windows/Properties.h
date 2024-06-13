//  This file is licenced under the GNU General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"
#include <unordered_map>

namespace Serialization{ struct KaraokeEffect; }
class PropertiesWindow : public EditorWindow
{
public:
    PropertiesWindow();
    void OnImGuiDraw() override;

private:
    enum TabIndex
    {
        DocumentTab,
        LocalTab
    };

    void DrawEffectWidget(std::string anEffectAlias, Serialization::KaraokeEffect* anEffect);
    void ApplyEdit(Serialization::KaraokeEffect* anEffect);

    TabIndex myCurrentTab;
    std::string myEditingEffect;
    std::string myNewEffectName;
    std::unordered_map<std::string, Serialization::KaraokeEffect*> myLocalEffectAliases;
};