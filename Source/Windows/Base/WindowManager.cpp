#include "WindowManager.h"
#include "EditorWindow.h"

	void WindowManager::Init()
	{
	}

	void WindowManager::StartFrame()
	{
		auto it = myWindows.end();
		while (it != myWindows.begin())
		{
			(--it)->second->StartFrame();
		}
	}

	void WindowManager::ImGuiDraw()
	{
		auto it = myWindows.end();
		while (it != myWindows.begin())
		{
			(--it)->second->OnImGuiDraw();
		}
		DestroyWindowInternal();
	}

	void WindowManager::EditorPlay()
	{
		auto it = myWindows.end();
		while (it != myWindows.begin())
		{
			(--it)->second->OnEditorPlay();
		}
	}

	void WindowManager::EditorPause()
	{
		auto it = myWindows.end();
		while (it != myWindows.begin())
		{
			(--it)->second->OnEditorPause();
		}
	}

	void WindowManager::EditorUnpause()
	{
		auto it = myWindows.end();
		while (it != myWindows.begin())
		{
			(--it)->second->OnEditorUnpause();
		}
	}

	void WindowManager::EditorStop()
	{
		auto it = myWindows.end();
		while (it != myWindows.begin())
		{
			(--it)->second->OnEditorStop();
		}
	}

	void WindowManager::EditorStep()
	{
		auto it = myWindows.end();
		while (it != myWindows.begin())
		{
			(--it)->second->OnEditorStep();
		}
	}

	EditorWindow* WindowManager::GetWindow(const std::string& aName)
	{
		if (myWindows.contains(aName))
		{
			return myWindows[aName];
		}
		//LOG_WARN("Window name doesn't exist!");
		return nullptr;
	}

	void WindowManager::DestroyWindow(EditorWindow* aWindow)
	{
		myWindowsToDelete.push_back(aWindow);
	}

	void WindowManager::DestroyWindowInternal()
	{
		for(EditorWindow* window : myWindowsToDelete)
		{
			if (window)
			{
				myWindows.erase(window->myName);
				delete window;
			}
		}
		myWindowsToDelete.clear();
	}
