#include "Base/EditorWindow.h"
#include <unordered_map>

namespace Serialization{ struct KaraokeEffect; }
class PropertiesWindow : public EditorWindow
{
public:
    void OnImGuiDraw() override;

private:
    enum TabIndex
    {
        DocumentTab,
        LocalTab
    };

    void DrawEffectWidget(std::string anEffectAlias);

    TabIndex myCurrentTab;
    std::unordered_map<std::string, Serialization::KaraokeEffect*> myLocalEffectAliases;
};