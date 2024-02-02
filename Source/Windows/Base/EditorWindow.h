#pragma once
#include <imgui/imgui.h>
#include "WindowManager.h"
//#include <CommandHistory.h>

//#define Gui_Begin(windowFlags) if(ImGui::Begin(GetName().c_str(), IsOpen(), windowFlags)){
//#define Gui_End() if(!*IsOpen()) ::Tools::WindowManager::DestroyWindow(this); ImGui::End();}

	class EditorWindow
	{
	public:
		EditorWindow() = default;
		virtual ~EditorWindow() = default;
		virtual void OnImGuiDraw() = 0;
		virtual void StartFrame() {}
		virtual void OnEditorPlay() {};
		virtual void OnEditorPause() {};
		virtual void OnEditorUnpause() {};
		virtual void OnEditorStop() {};
		virtual void OnEditorStep() {};

		std::string GetName() { return myName; }
		bool IsOpen() { return myIsOpen; }
		// RECT GetWindowRect();

	protected:
		bool Gui_Begin(ImGuiWindowFlags someWindowFlags = 0) { return ImGui::Begin(GetName().c_str(), &myIsOpen, someWindowFlags); }
		void Gui_End() { if (!myIsOpen) ::WindowManager::DestroyWindow(this); ImGui::End(); }

	private:
		friend class WindowManager;

		std::string myName;
		bool myIsOpen = true;
		// save window rect, set from windowManager.
	};