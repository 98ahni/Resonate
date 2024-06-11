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