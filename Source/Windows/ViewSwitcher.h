//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include "Base/EditorWindow.h"

class ViewSwitcher : public EditorWindow
{
public:
    ViewSwitcher();
    void OnImGuiDraw();
	template<typename Window>
	Window* AddWindow(std::string aName);
	EditorWindow* GetWindow(const std::string& aName);

private:
	std::map<std::string, EditorWindow*> myWindows;
	std::vector<std::string> myNames;
	std::string myCurrentView = "";
};

template<typename Window>
inline Window* ViewSwitcher::AddWindow(std::string aName)
{
	Window* aWindow = new Window();
	myWindows.insert({ aName, aWindow });
	myNames.push_back(aName);
	if(myCurrentView == "")
	{
		myCurrentView = aName;
	}
	return aWindow;
}