#pragma once
#include <webgpu/webgpu.h>

// Global WebGPU required states
struct ImFont;
struct MainWindow
{
	static inline WGPUDevice        Device = nullptr;
	inline static WGPUSurface       Surface = nullptr;
	static inline WGPUTextureFormat TextureFormat = WGPUTextureFormat_RGBA8Unorm;
	inline static WGPUSwapChain     SwapChain = nullptr;
	static inline int               SwapWidth = 0;
	inline static int               SwapHeight = 0;
	static inline bool				HasWebGPU = true;
	
	inline static ImFont*           Font = nullptr;
};

void MainWindow_Init(const char* name, void** outWindow);
void MainWindow_NewFrame(void* window);
void MainWindow_RenderFrame();
void MainWindow_StyleVarsShadow(struct ImGuiStyle* dst = nullptr);
void MainWindow_StyleColorsShadow(struct ImGuiStyle* dst = nullptr);