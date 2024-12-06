//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> (Used with permission from EODynamics AB) Original file author

#pragma once
#include <webgpu/webgpu.h>
#include <string>
#include <imgui.h>

// Global WebGPU required states
struct ImFont;
enum MainWindow_Platform { MainWindow_Unspecified = 0, MainWindow_Windows = 1<<0, MainWindow_Mac = 1<<1, MainWindow_iOS = 1<<2, MainWindow_Android = 1<<3, MainWindow_Linux = 1<<4, 
MainWindow_Apple = MainWindow_Mac | MainWindow_iOS};
struct MainWindow
{
	static inline WGPUDevice        Device = nullptr;
	inline static WGPUSurface       Surface = nullptr;
	static inline WGPUTextureFormat TextureFormat = WGPUTextureFormat_RGBA8Unorm;
	inline static WGPUSwapChain     SwapChain = nullptr;
	static inline int               SwapWidth = 0;
	inline static int               SwapHeight = 0;
	static inline bool				HasWebGPU = true;
	
	inline static double			DPIScale = 1;
	static inline MainWindow_Platform RuntimePlatform;
	inline static ImFont*           Font = nullptr;
	static inline std::string		Name = "";
	inline static std::string		IconPath = "";

	static inline ImVec2			DockSizeOffset = {0, 0};		// Space to reserve in the x/y axis for other content
};

void MainWindow_Init(const char* name, void** outWindow);
void MainWindow_NewFrame(void* window);
void MainWindow_RenderFrame();
void MainWindow_RenderCustomDrawData(ImDrawData* drawData, unsigned int aWidth, unsigned int aHeight);
void MainWindow_Invalidate();
bool MainWindow_IsPlatform(MainWindow_Platform platform);
void MainWindow_SetName(std::string name);
void MainWindow_SetIcon(std::string iconName);
void MainWindow_StyleVarsShadow(struct ImGuiStyle* dst = nullptr);
void MainWindow_StyleColorsShadow(struct ImGuiStyle* dst = nullptr);