//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#pragma once
#include <vector>
#include <map>
#include <string>

	class EditorWindow;
	class WindowManager
	{
	public:
		static void Init();
		template<typename Window>
		static Window* AddWindow(std::string aName);
		static void StartFrame();
		static void ImGuiDraw();
		static void EditorPlay();
		static void EditorPause();
		static void EditorUnpause();
		static void EditorStop();
		static void EditorStep();
		static EditorWindow* GetWindow(const std::string& aName);
		static void DestroyWindow(EditorWindow* aWindow);

	private:
		static inline std::map<std::string, EditorWindow*> myWindows;

		static void DestroyWindowInternal();
		static inline std::vector<EditorWindow*> myWindowsToDelete;
	};

	template<typename Window>
	inline Window* WindowManager::AddWindow(std::string aName)
	{
		Window* aWindow = new Window();
		aWindow->myName = aName;
		size_t postfix = 0;
		const std::string baseName = aWindow->myName;
		while (myWindows.contains(aWindow->myName))
		{
			++postfix;
			aWindow->myName = baseName + "##" + std::to_string(postfix);
		}
		myWindows.insert({ aWindow->myName, aWindow });
		return aWindow;
	}