//  This file is licenced under the GNU General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> (Used with permission from EODynamics AB) Original file author

#include "MainWindow.h"
#include <stdio.h>
#include <emscripten.h>
#include <emscripten/val.h>
#define GLFW_INCLUDE_ES32
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>
#include <backends/imgui_impl_opengl3.h>
#include <Extensions/TouchInput.h>
#include <Extensions/imguiExt.h>
#include <Defines.h>

/* embedded JS function to handle all the asynchronous WebGPU setup */
EM_ASYNC_JS(emscripten::EM_VAL, init_file_system, (), {
	return Emval.toHandle(new Promise((resolve) =>
	{
		FS.mount(MEMFS, { root: '.' }, '.');
		FS.mkdir('/local');
		FS.mount(IDBFS, {}, '/local');
    	FS.syncfs(true, function (err) {
			if(err)
			{
        		alert('Unable to sync IndexDB!\n' + err);
			}
			resolve();
    	});
	}));
});

EM_JS(bool, get_has_web_gpu, (), { 
  return navigator.gpu !== undefined;
	});

EM_ASYNC_JS(void, webgpu_create_device, (), {
	WebGPU.initManagers();
	console.log("Create Start!");
	const adapter = await navigator.gpu.requestAdapter();
	const device = await adapter.requestDevice();
	Module.preinitializedWebGPUDevice = device;
	console.log("Create End!");
});

EM_JS(void, auto_resize_canvas, (), {
	window.addEventListener('resize', resize_canvas, false);
});

EM_JS(void, resize_canvas, (), {
	document.getElementById('canvas').width = window.innerWidth;
	document.getElementById('canvas').height = window.innerHeight;
});

EM_JS(int, canvas_get_width, (), {
  return Module.canvas.width;
});

EM_JS(int, canvas_get_height, (), {
  return Module.canvas.height;
});

EM_JS(bool, has_fullscreen, (), {
    return (
      document.fullscreenEnabled || /* Standard syntax */
      document.webkitFullscreenEnabled || /* Safari and Opera syntax */
      document.msFullscreenEnabled /* IE11 syntax */
    );
});
EM_JS(bool, is_fullscreen, (), {
    if(document.fullscreenEnabled)
    {
        return document.fullscreenElement != null;
    }
    else if(document.webkitFullscreenEnabled) /* Safari */
    {
        return document.webkitFullscreenElement != null;
    }
    else if(document.msFullscreenEnabled) /* IE11 */
    {
        return document.msFullscreenElement != null;
    }
    return false;
});
EM_JS(void, open_fullscreen, (), {
    if(document.getElementById('canvas').requestFullscreen)
    {
        document.getElementById('canvas').requestFullscreen();
    }
    else if(document.getElementById('canvas').webkitRequestFullscreen) /* Safari */
    {
        document.getElementById('canvas').webkitRequestFullscreen();
    }
    else if(document.getElementById('canvas').msRequestFullscreen) /* IE11 */
    {
        document.getElementById('canvas').msRequestFullscreen();
    }
});
EM_JS(void, close_fullscreen, (), {
    if(document.exitFullscreen)
    {
        document.exitFullscreen();
    }
    else if(document.webkitExitFullscreen) /* Safari */
    {
        document.webkitExitFullscreen();
    }
    else if(document.msExitFullscreen) /* IE11 */
    {
        document.msExitFullscreen();
    }
});

static void print_wgpu_error(WGPUErrorType error_type, const char* message, void*)
{
	if(TouchInput_HasTouch())
	{
		EM_ASM(let errString = 'Undefined';
		if(error_type === 1) errString = 'Validation';
		else if(error_type === 2) errString = 'Out of memory';
		else if(error_type === 4) errString = 'Unknown';
		else if(error_type === 5) errString = 'Device lost';
		alert('WebGPU Error ' + errString);
		, error_type);
	}
	else
	{
		const char* error_type_lbl = "";
		switch(error_type)
		{
			case WGPUErrorType_Validation:  error_type_lbl = "Validation"; break;
			case WGPUErrorType_OutOfMemory: error_type_lbl = "Out of memory"; break;
			case WGPUErrorType_Unknown:     error_type_lbl = "Unknown"; break;
			case WGPUErrorType_DeviceLost:  error_type_lbl = "Device lost"; break;
			default:                        error_type_lbl = "Unknown";
		}
		printf("%s error: %s\n", error_type_lbl, message);
	}
}
static bool InitWGPU()
{
	if(!get_has_web_gpu()) return false;
	webgpu_create_device();
	printf("After!\n");
	MainWindow::Device = emscripten_webgpu_get_device();
	if(!MainWindow::Device)
		return false;

	wgpuDeviceSetUncapturedErrorCallback(MainWindow::Device, print_wgpu_error, nullptr);

	// Use C++ wrapper due to misbehavior in Emscripten.
	// Some offset computation for wgpuInstanceCreateSurface in JavaScript
	// seem to be inline with struct alignments in the C++ structure
	wgpu::SurfaceDescriptorFromCanvasHTMLSelector html_surface_desc = {};
	html_surface_desc.selector = "#canvas";

	wgpu::SurfaceDescriptor surface_desc = {};
	surface_desc.nextInChain = &html_surface_desc;

	wgpu::Instance instance = wgpuCreateInstance(nullptr);
	wgpu::Surface surface = instance.CreateSurface(&surface_desc);
	wgpu::Adapter adapter = {};
	MainWindow::TextureFormat = (WGPUTextureFormat)surface.GetPreferredFormat(adapter);
	MainWindow::Surface = surface.MoveToCHandle(); // has replaced Release();

	return true;
}

void MainWindow_Init(const char* name, void** outWindow)
{
	if(!glfwInit())
		return;
	
	// Make sure GLFW does not initialize any graphics context.
	// This needs to be done explicitly later.
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1280, 720, name, nullptr, nullptr);
	if(!window)
	{
		glfwTerminate();
		return;
	}

	// Initialize the WebGPU environment
	//if(true) // test WebGL
	if(!InitWGPU())
	{
		printf("WebGPU not supported!\n");
		//EM_ASM(alert('WebGPU not supported!'););
		MainWindow::HasWebGPU = false;
		if(window)
		{
			glfwDestroyWindow(window);
		}
		if(!glfwInit())
			return;
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL
		window = glfwCreateWindow(1280, 720, name, nullptr, nullptr);
		if(!window)
		{
			glfwTerminate();
			return;
		}
		glfwMakeContextCurrent(window);
	}
	glfwShowWindow(window);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;		// Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// Enable Docking

	// For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
	// You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
	//io.IniFilename = nullptr;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	if(MainWindow::HasWebGPU)
	{
		ImGui_ImplGlfw_InitForOther(window, false);
		*outWindow = window;
		ImGui_ImplWGPU_InitInfo info;
    	info.Device = MainWindow::Device;
    	info.NumFramesInFlight = 3;
    	info.RenderTargetFormat = MainWindow::TextureFormat;
    	info.DepthStencilFormat = WGPUTextureFormat_Undefined;
		ImGui_ImplWGPU_Init(&info);
		resize_canvas();
		auto_resize_canvas();
		ImGui_ImplWGPU_CreateDeviceObjects();
	}
	else
	{
		ImGui_ImplGlfw_InitForOpenGL(window, false);
		*outWindow = window;
		ImGui_ImplOpenGL3_Init("#version 300 es");
		resize_canvas();
		auto_resize_canvas();
	}

	VAR_FROM_JS(init_file_system()).await();

	//if(true) // debug touch
	if(TouchInput_HasTouch())
	{
		TouchInput_Init();
		glfwSetWindowFocusCallback(window, ImGui_ImplGlfw_WindowFocusCallback);
		glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
		glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
		glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
		glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);
	}
	else
	{
		ImGui_ImplGlfw_InstallCallbacks(window);
	}
	// Moved to imguiExt.cpp
	//io.GetClipboardTextFn = &GetClipboardContent;
	//io.SetClipboardTextFn = &SetClipboardContent;
}

void MainWindow_NewFrame(void* window)
{
	glfwPollEvents();
	int width, height;
	glfwGetFramebufferSize((GLFWwindow*)window, &width, &height);

	int canvasWidth = canvas_get_width();
	int canvasHeight = canvas_get_height();
	if(width != canvasWidth || height != canvasHeight)
	{
		printf("Resizing canvas\n");
		glfwSetWindowSize((GLFWwindow*)window, canvasWidth, canvasHeight);
		width = canvasWidth;
		height = canvasHeight;
	}

	// React to changes in screen size
	if(width != MainWindow::SwapWidth || height != MainWindow::SwapHeight)
	{
		if(MainWindow::HasWebGPU)
		{
			ImGui_ImplWGPU_InvalidateDeviceObjects();
			if(MainWindow::SwapChain)
				wgpuSwapChainRelease(MainWindow::SwapChain);
			MainWindow::SwapWidth = width;
			MainWindow::SwapHeight = height;
			WGPUSwapChainDescriptor swap_chain_desc = {};
			swap_chain_desc.usage = WGPUTextureUsage_RenderAttachment;
			swap_chain_desc.format = MainWindow::TextureFormat;
			swap_chain_desc.width = width;
			swap_chain_desc.height = height;
			swap_chain_desc.presentMode = WGPUPresentMode_Fifo;
			MainWindow::SwapChain = wgpuDeviceCreateSwapChain(MainWindow::Device, MainWindow::Surface, &swap_chain_desc);
			ImGui_ImplWGPU_CreateDeviceObjects();
		}
		else
		{
			MainWindow::SwapWidth = width;
			MainWindow::SwapHeight = height;
		}
	}

	if(!MainWindow::HasWebGPU)
		ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	if(MainWindow::HasWebGPU)
		ImGui_ImplWGPU_NewFrame();
	if(TouchInput_HasTouch()) { TouchInput_RunInput(); }
	ImGui::NewFrame();
	if(TouchInput_HasTouch()) { TouchInput_CheckKeyboard(); }   // Not working...

	ImVec2 vWindowSize = ImGui::GetMainViewport()->Size;
	ImVec2 vPos0 = ImGui::GetMainViewport()->Pos;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyle().Colors[ImGuiCol_Separator]);
	ImGui::SetNextWindowPos(ImVec2((float)vPos0.x, (float)vPos0.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2((float)vWindowSize.x, (float)vWindowSize.y), 1);
	if(ImGui::Begin(
		"EditorMain",
		/*p_open=*/nullptr,
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoTitleBar
		//| ImGuiWindowFlags_NoBackground
	))
	{
		static const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
		ImGuiID dockSpace = ImGui::GetID("EditorMain");
		ImGui::DockSpace(dockSpace, ImVec2(0.0f, 0.0f), dockspaceFlags);
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

void MainWindow_RenderFrame()
{
	ImGui::End();
	ImGui::Render();

	if(MainWindow::HasWebGPU)
	{
		WGPURenderPassColorAttachment color_attachments = {};
		color_attachments.loadOp = WGPULoadOp_Clear;
		color_attachments.storeOp = WGPUStoreOp_Store;
		color_attachments.clearValue = {1, 1, 1, 1};
		color_attachments.view = wgpuSwapChainGetCurrentTextureView(MainWindow::SwapChain);
		WGPURenderPassDescriptor render_pass_desc = {};
		render_pass_desc.colorAttachmentCount = 1;
		render_pass_desc.colorAttachments = &color_attachments;
		render_pass_desc.depthStencilAttachment = nullptr;

		WGPUCommandEncoderDescriptor enc_desc = {};
		WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(MainWindow::Device, &enc_desc);

		WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
		ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass);
		wgpuRenderPassEncoderEnd(pass);

		WGPUCommandBufferDescriptor cmd_buffer_desc = {};
		WGPUCommandBuffer cmd_buffer = wgpuCommandEncoderFinish(encoder, &cmd_buffer_desc);
		WGPUQueue queue = wgpuDeviceGetQueue(MainWindow::Device);
		wgpuQueueSubmit(queue, 1, &cmd_buffer);
	}
	else
	{
		glViewport(0, 0, MainWindow::SwapWidth, MainWindow::SwapHeight);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

void MainWindow_SetName(std::string name)
{
    if(MainWindow::Name == name) return;
    MainWindow::Name = name;
    emscripten_set_window_title(MainWindow::Name.data());
}

void MainWindow_SetIcon(std::string iconName)
{
    if(MainWindow::IconPath == iconName) return;
    MainWindow::IconPath = iconName;
	//EM_ASM({action.setIcon({path: {32: Emval.toValue($0)}})}, VAR_TO_JS(MainWindow::IconPath));
	EM_ASM({
		if(!document.querySelector("link[rel='icon']"))
		{
			let link = document.createElement('link');
			link.rel = 'icon';
			link.type = 'image/png';
			document.head.appendChild(link);
		}
		document.querySelector("link[rel='icon']").href = "icons/" + Emval.toValue($0);
	}, VAR_TO_JS(MainWindow::IconPath));
}

void MainWindow_StyleVarsShadow(ImGuiStyle* dst)
{
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();

	style->ScrollbarRounding = 2;
	style->WindowBorderSize = 2;
	style->WindowMenuButtonPosition = ImGuiDir_None;
	style->WindowRounding = 1;
	style->ItemSpacing = {10, 5};
	style->FrameBorderSize = 0;
	style->FramePadding = {15, 3};
	style->FrameRounding = 2;
	style->PopupRounding = 2;
	if(TouchInput_HasTouch())
	{
		style->ScaleAllSizes(1.5f);
		style->TouchExtraPadding = {5, 5};
	}
	style->ScrollbarSize = 15;
}

void MainWindow_StyleColorsShadow(ImGuiStyle* dst)
{
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImColor* colors = (ImColor*)style->Colors;

	ImVec4 text = ImVec4(1.f, 1.f, 1.f, 1.00f);
	ImVec4 bg = ImVec4(0.2f, 0.2f, 0.22f, 1.f);
	ImVec4 darkbg = ImVec4(0.09f, 0.08f, 0.12f, 1.f);
	ImVec4 lightItembg = ImVec4(0.3f, 0.3f, 0.3f, 1.f);
	ImVec4 itembg = ImVec4(0.1f, 0.1f, 0.1f, 1.f);
	ImVec4 lightItemHover = ImVec4(0.37f, 0.35f, 0.4f, 1.f);
	ImVec4 itemHover = ImVec4(0.15f, 0.13f, 0.2f, 1.f);
	ImVec4 lightItemActive = ImVec4(0.37f, 0.35f, 0.4f, 1.f);
	ImVec4 itemActive = ImVec4(0.15f, 0.15f, 0.2f, 1.f);

	colors[ImGuiCol_Text] = text;
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg] = bg;
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = darkbg;
	colors[ImGuiCol_Border] = darkbg;
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
	colors[ImGuiCol_FrameBg] = itembg;
	colors[ImGuiCol_FrameBgHovered] = itemHover;
	colors[ImGuiCol_FrameBgActive] = itemActive;
	colors[ImGuiCol_TitleBg] = darkbg;
	colors[ImGuiCol_TitleBgActive] = colors[ImGuiCol_TitleBg];
	colors[ImGuiCol_TitleBgCollapsed] = colors[ImGuiCol_TitleBg];
	colors[ImGuiCol_MenuBarBg] = colors[ImGuiCol_TitleBg];//ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = darkbg;
	colors[ImGuiCol_ScrollbarGrab] = lightItembg;
	colors[ImGuiCol_ScrollbarGrabHovered] = lightItemHover;
	colors[ImGuiCol_ScrollbarGrabActive] = lightItemActive;
	colors[ImGuiCol_CheckMark] = text;
	colors[ImGuiCol_SliderGrab] = lightItembg;
	colors[ImGuiCol_SliderGrabActive] = lightItemActive;
	colors[ImGuiCol_Button] = lightItembg;
	colors[ImGuiCol_ButtonHovered] = lightItemHover;
	colors[ImGuiCol_ButtonActive] = lightItemActive;
	colors[ImGuiCol_Header] = itembg;
	colors[ImGuiCol_HeaderHovered] = itemHover;
	colors[ImGuiCol_HeaderActive] = itemActive;
	colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.09f, 0.12f, 1.f);
	colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_TabActive] = itemActive;
	colors[ImGuiCol_TabUnfocused] = colors[ImGuiCol_Tab];
	colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TitleBg];
	colors[ImGuiCol_DockingPreview] = colors[ImGuiCol_HeaderActive];
	colors[ImGuiCol_DockingPreview].Value.w = .70f;
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = lightItemActive;
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}
