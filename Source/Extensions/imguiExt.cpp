//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imguiExt.h"
#include "imguiExtTexture.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <Serialization/KaraokeData.h>
#include <Windows/MainWindow.h>
#include <Defines.h>
#include <emscripten/html5_webgpu.h>
#define GLFW_INCLUDE_ES32
#include <GLES3/gl3.h>

EM_JS(void, create_button, (emscripten::EM_VAL id, emscripten::EM_VAL event, emscripten::EM_VAL callback, int pos_x, int pos_y, int width, int height), {
    let imid = Emval.toValue(id);
    let btn = document.getElementById(imid);
    if(btn === null){
        btn = document.createElement('button');
        btn.id = imid;
        document.body.insertBefore(btn, document.getElementById('canvas').nextSibling);
    }
    btn.addEventListener(Emval.toValue(event), window[Emval.toValue(callback)], false);
    btn.style.position = 'fixed';
    btn.style.left = pos_x + 'px';
    btn.style.top = pos_y + 'px';
    btn.style.width = width + 'px';
    btn.style.height = height + 'px';
    btn.style.opacity = 0.1;
});
EM_JS(void, create_input, (emscripten::EM_VAL id, emscripten::EM_VAL type, emscripten::EM_VAL event, emscripten::EM_VAL callback, int pos_x, int pos_y, int width, int height), {
    let imid = Emval.toValue(id);
    let input = document.getElementById(imid);
    if(input === null){
        input = document.createElement('input');
        input.id = imid;
        document.body.insertBefore(input, document.getElementById('canvas').nextSibling);
    }
    input.addEventListener(Emval.toValue(event), window[Emval.toValue(callback)], true);
    input.type = Emval.toValue(type);
    input.style.position = 'fixed';
    input.style.left = pos_x + 'px';
    input.style.top = pos_y + 'px';
    input.style.width = width + 'px';
    input.style.height = height + 'px';
    input.style.opacity = 0;
});
EM_JS(emscripten::EM_VAL, load_video, (emscripten::EM_VAL id, emscripten::EM_VAL fs_path), {
    return Emval.toHandle(new Promise(async(resolve, reject)=>{
    var imid = Emval.toValue(id);
    var vid = document.getElementById(imid);
    const fsPath = Emval.toValue(fs_path);
    if(vid === null){
        if(!FS.analyzePath(fsPath, false).exists){
            console.error('File not found: ', fsPath);
            reject();
        }
        vid = document.createElement('video');
        vid.id = imid;
        vid.volume = 0;             // If the muted and volume settings are not set here AND after the source has been set
        vid.defaultMuted = true;        // videos that contain audio won't play on iOS for some reason. :')
        document.body.insertBefore(vid, document.getElementById('canvas'));
    }
    vid.style.position = 'fixed';
    vid.style.width = 160 + 'px';
    vid.style.height = 90 + 'px';
	const vidData = FS.readFile(fsPath);
    const vidBlob = new Blob([vidData.buffer], {type: 'video/mp4'});
    //const vidSource = new (window.ManagedMediaSource || window.MediaSource)();
    vid.src = URL.createObjectURL(vidBlob);
    //vid.autoplay="autoplay";
    vid.disablePictureInPicture = true;
    vid.volume = 0;
    vid.defaultMuted = true;
    //vidSource.addEventListener("sourceopen", async ()=>{
    //    const sourceBuff = vidSource.addSourceBuffer("video/mp4; codecs=\"avc1.4d002a\"");
    //    const arr = await vidBlob.arrayBuffer();
    //    sourceBuff.appendBuffer(arr);
    //    vid.play();
    //});
    vid.load();
    vid.oncanplaythrough = ()=>{vid.play().then(()=>{vid.pause();});resolve();};
    }));
});
EM_JS(void, play_video, (emscripten::EM_VAL id), {
    let imid = Emval.toValue(id);
    let vid = document.getElementById(imid);
    if(vid === null){
        return;
    }
    vid.play();
});
EM_JS(void, pause_video, (emscripten::EM_VAL id), {
    let imid = Emval.toValue(id);
    let vid = document.getElementById(imid);
    if(vid === null){
        return;
    }
    vid.pause();
});
EM_JS(void, set_video_playback_progress, (emscripten::EM_VAL id, double seconds), {
    let imid = Emval.toValue(id);
    let vid = document.getElementById(imid);
    if(vid === null){
        return;
    }
    vid.currentTime = seconds;
});
EM_JS(void, set_video_playback_rate, (emscripten::EM_VAL id, double rate), {
    let imid = Emval.toValue(id);
    let vid = document.getElementById(imid);
    if(vid === null){
        return;
    }
    vid.playbackRate = rate;
});
EM_JS(bool, is_video_paused, (emscripten::EM_VAL id), {
    let imid = Emval.toValue(id);
    let vid = document.getElementById(imid);
    if(vid === null){
        return true;
    }
    return vid.paused;
});
EM_JS(emscripten::EM_VAL, load_image_from_url, (emscripten::EM_VAL id, emscripten::EM_VAL url), {
    return Emval.toHandle(new Promise(async(resolve)=>{
    let imid = Emval.toValue(id);
    let img = document.getElementById(imid);
    const imgURL = Emval.toValue(url);
    if(img === null){
        img = document.createElement('img');
        img.id = imid;
        img.crossOrigin = "anonymous";
        img.referrerPolicy = 'origin';
        document.body.insertBefore(img, document.getElementById('canvas'));
    }
    img.style.position = 'fixed';
    img.style.width = 1 + 'px';
    img.style.height = 1 + 'px';
    img.src = imgURL;
    try{
        await img.decode();
        resolve();
    }
    catch(e){
        console.error('Image failed to decode!');
        resolve();
    }
    }));
});
EM_JS(emscripten::EM_VAL, load_image, (emscripten::EM_VAL id, emscripten::EM_VAL fs_path), {
    return Emval.toHandle(new Promise(async(resolve, reject)=>{
    let imid = Emval.toValue(id);
    let img = document.getElementById(imid);
    const fsPath = Emval.toValue(fs_path);
    if(img === null){
        if(!FS.analyzePath(fsPath, false).exists){
            console.error('File not found: ', fsPath);
            reject();
        }
    }
	const imgData = FS.readFile(fsPath);
    const imgBlob = new Blob([imgData.buffer], {type: 'application/octet-binary'});
    Emval.toValue(load_image_from_url(Emval.toHandle(imid), Emval.toHandle(URL.createObjectURL(imgBlob)))).then(resolve, reject);
    }));
});
EM_JS(ImExtTexture&, render_image, (emscripten::EM_VAL id, ImExtTexture& texture), {
    var imid = Emval.toValue(id);
    var img = document.getElementById(imid);
    if(img === null){
        return Emval.toHandle(0);
    }
    var canvas = document.getElementById(imid + 'canvas');
    if(canvas === null){
        canvas = document.createElement('canvas');
        canvas.id = imid + 'canvas';
        document.body.insertBefore(canvas, document.getElementById('canvas'));
        if(img.nodeName == 'IMG'){
            canvas.width = img.naturalWidth;
            canvas.height = img.naturalHeight;
        }
        else if(img.nodeName == 'VIDEO'){
            canvas.width = img.videoWidth;
            canvas.height = img.videoHeight;
        }
        canvas.style.position = 'fixed';
        canvas.style.top = 100 + 'px';
        canvas.style.width = 160 + 'px';
        canvas.style.height = 90 + 'px';
    }
    var ctx = canvas.getContext('2d');
    ctx.drawImage(img, 0, 0);
    var pixels = ctx.getImageData(0, 0, canvas.width, canvas.height);
    var output = _CreateTexture(texture, Emval.toHandle(pixels.data), canvas.width, canvas.height);
    ctx = null;
    pixels = null;
    return output;
});
EM_JS(void, destroy_element, (emscripten::EM_VAL id), {
    let input = document.getElementById(Emval.toValue(id));
    if(input !== null){
        input.remove();
    }
});
EM_ASYNC_JS(void, destroy_element_async, (emscripten::EM_VAL id, int delay_ms), {
    let input = document.getElementById(Emval.toValue(id));
    setTimeout(()=>{
        if(input !== null){
            input.remove();
        }
    }, delay_ms);
});
EM_JS(void, add_window_event, (emscripten::EM_VAL event, emscripten::EM_VAL callback), {
    window.addEventListener(Emval.toValue(event), window[Emval.toValue(callback)], true);
});
EM_JS(void, remove_window_event, (emscripten::EM_VAL event, emscripten::EM_VAL callback), {
    window.removeEventListener(Emval.toValue(event), window[Emval.toValue(callback)], true);
});

ImExtTexture CreateTextureWebGPU(ImExtTexture texture, void* textureBytes, unsigned int sizeX, unsigned int sizeY)
{
    //if(texture)
    //{
    //    wgpuTextureViewRelease((WGPUTextureView)texture);
    //}
    ImTextureID output = texture.myID;
	ImTextureID Texture = texture.myHandle;

    if(!texture.myHandle)
    {
	    WGPUTextureDescriptor textureDesc = {};
	    textureDesc.nextInChain = nullptr;
	    textureDesc.dimension = WGPUTextureDimension_2D;
	    textureDesc.size = {sizeX, sizeY, 1};
	    textureDesc.mipLevelCount = 1;
	    textureDesc.sampleCount = 1;
	    textureDesc.format = WGPUTextureFormat_RGBA8Unorm;
	    textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
	    textureDesc.viewFormatCount = 0;
	    textureDesc.viewFormats = nullptr;
	    Texture = wgpuDeviceCreateTexture(MainWindow::Device, &textureDesc);
    }
    
	WGPUImageCopyTexture destination = {};
	destination.texture = (WGPUTexture)Texture;
	destination.mipLevel = 0;
	destination.origin = {0, 0, 0};
	destination.aspect = WGPUTextureAspect_All;
	WGPUTextureDataLayout source = {};
	source.offset = 0;
	source.bytesPerRow = 4 * sizeX;
	source.rowsPerImage = sizeY;
    WGPUExtent3D writeSize = {sizeX, sizeY, 1};
    WGPUQueue deviceQueue =  wgpuDeviceGetQueue(MainWindow::Device);
	wgpuQueueWriteTexture(deviceQueue, &destination, textureBytes, sizeX * 4 * sizeY, &source, &writeSize);
    
    if(!texture.myID)
    {
	    WGPUTextureViewDescriptor textureViewDesc = {};
	    textureViewDesc.format = WGPUTextureFormat_RGBA8Unorm;
	    textureViewDesc.dimension = WGPUTextureViewDimension_2D;
	    textureViewDesc.baseMipLevel = 0;
	    textureViewDesc.mipLevelCount = 1;
	    textureViewDesc.baseArrayLayer = 0;
	    textureViewDesc.arrayLayerCount = 1;
	    textureViewDesc.aspect = WGPUTextureAspect_All;
        output = wgpuTextureCreateView((WGPUTexture)Texture, &textureViewDesc);
    }

    //wgpuTextureRelease((WGPUTexture)Texture);
    wgpuQueueRelease(deviceQueue);

    return {output, Texture};
}
ImExtTexture CreateTextureWebGL(ImExtTexture texture, void* textureBytes, unsigned int sizeX, unsigned int sizeY)
{
	GLint last_texture;
	GLuint outTexture = (GLuint)(intptr_t)texture.myID;

	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    if(texture.myID == 0)
    {
	    glGenTextures(1, &outTexture);
    }
	glBindTexture(GL_TEXTURE_2D, outTexture);
    if(texture.myID == 0)
    {
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
	    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    }

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureBytes);

	glBindTexture(GL_TEXTURE_2D, last_texture);

	return {(ImTextureID)(intptr_t)outTexture};
}
extern"C" EMSCRIPTEN_KEEPALIVE const ImExtTexture& CreateTexture(const ImExtTexture& texture, emscripten::EM_VAL byteArray, unsigned int sizeX, unsigned int sizeY)
{
    //std::vector<unsigned char> textureBytes = emscripten::vecFromJSArray<unsigned char>(VAR_FROM_JS(byteArray));
    std::string textureBytes = VAR_FROM_JS(byteArray).as<std::string>();
	if(MainWindow::HasWebGPU)
	{
		return CreateTextureWebGPU(texture, textureBytes.data(), sizeX, sizeY);
	}
	return CreateTextureWebGL(texture, textureBytes.data(), sizeX, sizeY);
}

void ImGui::Ext::CreateHTMLButton(const char *anID, const char *anEvent, const char *aJSFunctonName)
{
    ImGuiWindow* window = GetCurrentContext()->CurrentWindow;
    ImVec4 clipRect = 
    {
        std::max(ImGui::GetItemRectMin().x, GetWindowPos().x),
        std::max(ImGui::GetItemRectMin().y, GetWindowPos().y),
        std::min(ImGui::GetItemRectMax().x, GetWindowPos().x + (GetWindowWidth() - window->ScrollbarSizes.x)),
        std::min(ImGui::GetItemRectMax().y, GetWindowPos().y + (GetWindowHeight() - window->ScrollbarSizes.y))
    };
    if(clipRect.z < clipRect.x || clipRect.w < clipRect.y || !IsItemVisible())
    {
        DestroyHTMLElement(anID);
        return;
    }
    create_button(emscripten::val(anID).as_handle(), emscripten::val(anEvent).as_handle(), emscripten::val(aJSFunctonName).as_handle(), (int)DPI_UNSCALED(clipRect.x), (int)DPI_UNSCALED(clipRect.y), (int)DPI_UNSCALED(clipRect.z - clipRect.x), (int)DPI_UNSCALED(clipRect.w - clipRect.y));
}

void ImGui::Ext::CreateHTMLButton(ImVec2 aPosition, ImVec2 aSize, const char *anID, const char *anEvent, const char *aJSFunctonName)
{
    create_button(emscripten::val(anID).as_handle(), emscripten::val(anEvent).as_handle(), emscripten::val(aJSFunctonName).as_handle(), (int)DPI_UNSCALED(aPosition.x), (int)DPI_UNSCALED(aPosition.y), (int)DPI_UNSCALED(aSize.x), (int)DPI_UNSCALED(aSize.y));
}

void ImGui::Ext::CreateHTMLInput(const char *anID, const char *aType, const char *anEvent, const char *aJSFunctonName)
{
    ImGuiWindow* window = GetCurrentContext()->CurrentWindow;
    ImVec4 clipRect = 
    {
        std::max(ImGui::GetItemRectMin().x, GetWindowPos().x),
        std::max(ImGui::GetItemRectMin().y, GetWindowPos().y),
        std::min(ImGui::GetItemRectMax().x, GetWindowPos().x + (GetWindowWidth() - window->ScrollbarSizes.x)),
        std::min(ImGui::GetItemRectMax().y, GetWindowPos().y + (GetWindowHeight() - window->ScrollbarSizes.y))
    };
    if(clipRect.z < clipRect.x || clipRect.w < clipRect.y || !IsItemVisible())
    {
        DestroyHTMLElement(anID);
        return;
    }
    create_input(emscripten::val(anID).as_handle(), emscripten::val(aType).as_handle(), emscripten::val(anEvent).as_handle(), emscripten::val(aJSFunctonName).as_handle(), (int)DPI_UNSCALED(ImGui::GetItemRectMin().x), (int)DPI_UNSCALED(ImGui::GetItemRectMin().y), (int)DPI_UNSCALED(ImGui::GetItemRectSize().x), (int)DPI_UNSCALED(ImGui::GetItemRectSize().y));
}

void ImGui::Ext::CreateHTMLInput(ImVec2 aPosition, ImVec2 aSize, const char *anID, const char *aType, const char *anEvent, const char *aJSFunctonName)
{
    create_input(emscripten::val(anID).as_handle(), emscripten::val(aType).as_handle(), emscripten::val(anEvent).as_handle(), emscripten::val(aJSFunctonName).as_handle(), (int)DPI_UNSCALED(aPosition.x), (int)DPI_UNSCALED(aPosition.y), (int)DPI_UNSCALED(aSize.x), (int)DPI_UNSCALED(aSize.y));
}

void ImGui::Ext::DestroyHTMLElement(const char *anID, int aDelayMillis)
{
    if(aDelayMillis > 0)
    {
        destroy_element_async(emscripten::val(anID).as_handle(), aDelayMillis);
    }
    else
    {
        destroy_element(emscripten::val(anID).as_handle());
    }
}

void ImGui::Ext::AddWindowEvent(const char *anEvent, const char *aJSFunctionName)
{
    add_window_event(VAR_TO_JS(anEvent), VAR_TO_JS(aJSFunctionName));
}

void ImGui::Ext::RemoveWindowEvent(const char *anEvent, const char *aJSFunctionName)
{
    remove_window_event(VAR_TO_JS(anEvent), VAR_TO_JS(aJSFunctionName));
}

void ImGui::Ext::LoadVideo(const char *anID, const char *anFSPath)
{
    VAR_FROM_JS(load_video(VAR_TO_JS(anID), VAR_TO_JS(anFSPath))).await();
}

void ImGui::Ext::PlayVideo(const char *anID)
{
    play_video(VAR_TO_JS(anID));
}

void ImGui::Ext::PauseVideo(const char *anID)
{
    pause_video(VAR_TO_JS(anID));
}

void ImGui::Ext::SetVideoProgress(const char *anID, uint aProgress)
{
    set_video_playback_progress(VAR_TO_JS(anID), (double)(aProgress) * .01);
}

void ImGui::Ext::SetVideoSpeed(const char *anID, int aSpeed)
{
    set_video_playback_rate(VAR_TO_JS(anID), (double)(aSpeed) * .1);
}

bool ImGui::Ext::IsVideoPaused(const char *anID)
{
    return is_video_paused(VAR_TO_JS(anID));
}

void ImGui::Ext::LoadImage(const char *anID, const char *anFSPath)
{
    VAR_FROM_JS(load_image(VAR_TO_JS(anID), VAR_TO_JS(anFSPath))).await();
}

void ImGui::Ext::LoadImageFromURL(const char *anID, const char *aURL)
{
    VAR_FROM_JS(load_image_from_url(VAR_TO_JS(anID), VAR_TO_JS(aURL))).await();
}

bool ImGui::Ext::RenderTexture(const char *anID, ImExtTexture& aTexture)
{
    aTexture = render_image(VAR_TO_JS(anID), aTexture);
    return aTexture.myID != 0;
}
bool ImGui::Ext::DeleteTexture(const char *anID, ImExtTexture &aTexture)
{
    if(aTexture.myID == 0)
    {
        return false;
    }
	if(MainWindow::HasWebGPU)
    {
        if(aTexture.myID)
        {
            wgpuTextureViewRelease((WGPUTextureView)aTexture.myID);
        }
        if(aTexture.myHandle)
        {
            wgpuTextureRelease((WGPUTexture)aTexture.myHandle);
        }
    }
    else
    {
        glDeleteTextures(1, (GLuint*)(intptr_t*)&aTexture.myID);
    }
    DestroyHTMLElement((std::string(anID) + "canvas").c_str());
    DestroyHTMLElement(anID);
    return true;
}
#define GetClipboardAction() ((ClipboardAction)(int)ImGui::GetIO().ClipboardUserData)
#define SetClipboardAction(value) ImGui::GetIO().ClipboardUserData = (void*)(int)(value)
enum ClipboardAction
{
    none, paste, copy, cut
};
EM_ASYNC_JS(emscripten::EM_VAL, get_clipboard_content, (), {
	//const output = await new Promise((resolve)=>{navigator.clipboard.readText().then((text)=>{resolve(text);});});
	var output = '';
	const clipboardContents = await navigator.clipboard.read();
    for (const item of clipboardContents) {
		if (item.types.includes("text/plain")) {
    		let blob = await item.getType("text/plain");
    		output = await blob.text();
			//return Emval.toHandle(output);
		}
	}
	return Emval.toHandle(output);
});
extern"C" EMSCRIPTEN_KEEPALIVE void GetClipboardContent()
{
	static std::string output = "";
    if(ImGui::GetIO().WantTextInput)
    {
	    output = VAR_FROM_JS(get_clipboard_content()).as<std::string>().c_str();
        ImGui::GetCurrentContext()->ClipboardHandlerData.clear();
        ImGui::GetCurrentContext()->ClipboardHandlerData.reserve(output.size());
        for(int i = 0; i < output.size(); i++)
        {
            ImGui::GetCurrentContext()->ClipboardHandlerData.push_back(output[i]);
        }
        SetClipboardAction(paste);
    }
}

EM_ASYNC_JS(void, set_clipboard_content, (emscripten::EM_VAL content), {
	const type = "text/plain";
  	const blob = new Blob([Emval.toValue(content)], { type });
  	const data = [new ClipboardItem({ [type]: blob })];
  	await navigator.clipboard.write(data);
});
void SetClipboardContent(bool aShouldCut)
{
    SetClipboardAction(aShouldCut ? ClipboardAction::cut : ClipboardAction::copy);
    //ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetActiveID());
    //if(ImGui::GetIO().WantTextInput && state)
    //{
    //    std::string content = std::string(state->TextA.Data).substr(state->GetSelectionStart(), state->GetSelectionEnd());
    //    printf("copying: %s\n", content.data());
	//    set_clipboard_content(VAR_TO_JS(content.c_str()));
    //    if(aShouldCut)
    //    {
    //        state->ClearSelection();
    //    }
    //}
}
extern"C" EMSCRIPTEN_KEEPALIVE void CopyClipboardContent()
{
    SetClipboardContent(false);
}
extern"C" EMSCRIPTEN_KEEPALIVE void CutClipboardContent()
{
    SetClipboardContent(true);
}

void ImGui::Ext::SetShortcutEvents()
{
    SetClipboardAction(none);
    ImGui::GetIO().SetClipboardTextFn = [](void* data, const char* text){
            //ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetActiveID());
            //if(ImGui::GetIO().WantTextInput && state && state->HasSelection())
            //{
            //    std::string content = "";
            //    content.resize(state->TextA.size());
            //    int start = (state->GetSelectionStart() < state->GetSelectionEnd() ? state->GetSelectionStart() : state->GetSelectionEnd());
            //    int end = (state->GetSelectionStart() > state->GetSelectionEnd() ? state->GetSelectionStart() : state->GetSelectionEnd());
            //    std::memcpy(content.data(), state->TextA.Data + start, end - start);
            //    printf("copying: '%s' | valid: %s\n", content.data(), state->TextAIsValid ? "true" : "false");
	        //    set_clipboard_content(VAR_TO_JS(content.c_str()));
            //}
            SetClipboardAction(none);
        };
    ImGui::GetIO().GetClipboardTextFn = [](void* data)->const char*{
	        printf("Pasting '%s'\n", ImGui::GetCurrentContext()->ClipboardHandlerData.Data);
            ImGui::GetCurrentContext()->ClipboardHandlerData.push_back('\0');
            SetClipboardAction(none);
            return ImGui::GetCurrentContext()->ClipboardHandlerData.Data;
        };
    AddWindowEvent("copy", "_CopyClipboardContent");
    AddWindowEvent("cut", "_CutClipboardContent");
    AddWindowEvent("paste", "_GetClipboardContent");
    ImGuiContextHook copyPaste;
    copyPaste.Type = ImGuiContextHookType_NewFramePre;
    copyPaste.Callback = [](ImGuiContext *ctx, ImGuiContextHook *hook){
        if(GetClipboardAction() == ClipboardAction::paste && ImGui::GetCurrentContext()->ClipboardHandlerData.size() != 0)
        {
	        //printf("Pasting '%s'\n", ImGui::GetCurrentContext()->ClipboardHandlerData.Data);
            //ImGui::GetCurrentContext()->ClipboardHandlerData.push_back('\0');
            //ImGui::GetIO().AddInputCharactersUTF8(ImGui::GetCurrentContext()->ClipboardHandlerData.Data);
            //ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetActiveID());
            //if(state)
            //{
            //    state->Edited = true;
            //    state->Flags &= ImGuiInputTextFlags_CallbackEdit;
            //}
            ////ImGui::GetCurrentContext()->ClipboardHandlerData.clear();
            //SetClipboardAction(none);
        }
        else if((GetClipboardAction() == ClipboardAction::copy || GetClipboardAction() == ClipboardAction::cut))
        {
            ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetActiveID());
            if(ImGui::GetIO().WantTextInput && state && state->HasSelection())
            {
                std::string content = "";
                content.resize(state->TextA.size());
                int start = (state->GetSelectionStart() < state->GetSelectionEnd() ? state->GetSelectionStart() : state->GetSelectionEnd());
                int end = (state->GetSelectionStart() > state->GetSelectionEnd() ? state->GetSelectionStart() : state->GetSelectionEnd());
                std::memcpy(content.data(), state->TextA.Data + start, end - start);
	            set_clipboard_content(VAR_TO_JS(content.c_str()));
                printf("copying: '%s' | valid: %s\n", content.data(), state->TextAIsValid ? "true" : "false");
                //if(GetClipboardAction() == ClipboardAction::cut)
                //{
                //    state->ClearSelection();
                //}
            }
            //SetClipboardAction(none);
        }
    };
    ImGui::AddContextHook(ImGui::GetCurrentContext(), &copyPaste);
}

void ImGui::Ext::StartLoadingScreen(float someAlpha, bool aShouldAnimateIn)
{
    EM_ASM(Module.show_loading_screen($0, $1, $2);, ImGui::GetColorU32(ImGuiCol_PopupBg, someAlpha), ImGui::GetColorU32(ImGuiCol_ButtonActive), aShouldAnimateIn);
}

void ImGui::Ext::StopLoadingScreen()
{
    EM_ASM(Module.hide_loading_screen(););
}

bool ImGui::Ext::TimedSyllable(std::string aValue, uint aStartTime, uint anEndTime, uint aCurrentTime, bool aShowProgress, bool aFlashToken, bool aUseAlpha, float anOutlineSize, float aMaxGrowFactor)
{
    ImVec2 size = CalcTextSize(aValue.data());
    ImVec2 pos = GetCursorScreenPos();
    float start = aStartTime;
    float end = anEndTime;
    ImVec2 timeStartPos = {pos.x, pos.y + (size.y * 1.1f)};
    ImVec2 timeEndPos = {remap(clamp((float)aCurrentTime, start, end), start, end, pos.x, pos.x + size.x), pos.y + (size.y * 1.1f)};
    float scale = aMaxGrowFactor == 1 ? 1 : remap(sinf(remap(clamp((float)aCurrentTime, start, end), start, end, 0, 3.14159)), 0, 1, 1, aMaxGrowFactor);
    pos.x -= (size.x * (scale - 1) * .5f);
    pos.y -= (size.y * (scale - 1) * .7f);
    ImDrawList* drawList = GetWindowDrawList();
    //static ImDrawList* drawList = new ImDrawList(GetDrawListSharedData());
    //drawList->AddDrawCmd();
    //drawList->PushTextureID(GetFont()->ContainerAtlas->TexID);
    if(anOutlineSize != 0 && aCurrentTime < end)
    {
        //ImDrawList* drawList = GetWindowDrawList();
        for(int i = 0; i < 5; i++)
        {
            drawList->AddText(GetFont(), size.y /*GetFont()->FontSize * GetFont()->Scale*/ * scale, {pos.x + (cosf(i * (6.28318 * .2f)) * anOutlineSize), pos.y + (sinf(i * (6.28318 * .2f)) * anOutlineSize)}, IM_COL32(0, 0, 0, 155), aValue.data());
            //drawList->AddText({(cosf(i * (6.28318 * .2f)) * anOutlineSize), (sinf(i * (6.28318 * .2f)) * anOutlineSize)}, IM_COL32(0, 0, 0, 155), aValue.data());
        }
    }
    if(aCurrentTime < start)
    {
        uint startCol = Serialization::KaraokeDocument::Get().GetStartColor();
        startCol = IM_COL32_FROM_DOC(startCol) | (aUseAlpha ? 0 : 0xFF000000);
        //ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_FROM_DOC(startCol) | (aUseAlpha ? 0 : 0xFF000000));
        //drawList->AddText({0, 0}, IM_COL32_FROM_DOC(startCol) | (aUseAlpha ? 0 : 0xFF000000), aValue.data());
        drawList->AddText(GetFont(), size.y /*GetFont()->FontSize * GetFont()->Scale*/, pos, startCol, aValue.data());
    }
    else if(aCurrentTime < end)
    {
        uint startCol = Serialization::KaraokeDocument::Get().GetStartColor();
        startCol = IM_COL32_FROM_DOC(startCol) | (aUseAlpha ? 0 : 0xFF000000);
        uint endCol = Serialization::KaraokeDocument::Get().GetEndColor();
        endCol = IM_COL32_FROM_DOC(endCol) | (aUseAlpha ? 0 : 0xFF000000);
        if(aFlashToken || startCol == endCol)
        {
            startCol = IM_COL32(
                (IM_COL32_GET_R(startCol) < 200 ? 255 : 155),
                (IM_COL32_GET_G(startCol) < 200 ? 255 : 155),
                (IM_COL32_GET_B(startCol) < 200 ? 255 : 155),
                255
            );
            //if(IM_COL32_FROM_DOC(startCol) == IM_COL32_WHITE)
            //{
            //    startCol = IM_COL32(210, 190, 255, 255);
            //}
            //else
            //{
            //    startCol = IM_COL32_WHITE;
            //}
        }
        ImVec4 startColClip = {timeEndPos.x, 0, FLT_MAX, FLT_MAX};
        ImVec4 endColClip = {0, 0, timeEndPos.x, FLT_MAX};
        drawList->AddText(GetFont(), size.y /*GetFont()->FontSize * GetFont()->Scale*/ * scale, pos, startCol, aValue.data(), nullptr, 0, &startColClip);
        drawList->AddText(GetFont(), size.y /*GetFont()->FontSize * GetFont()->Scale*/ * scale, pos, endCol, aValue.data(), nullptr, 0, &endColClip);
        //if(IM_COL32_FROM_DOC(startCol) == IM_COL32_WHITE)
        //{
        //    ImGui::PushStyleColor(ImGuiCol_Text, {0.87f, 0.8f, 1.f, 1.f});
        //    //drawList->AddText({0, 0}, IM_COL32(210, 190, 255, 255), aValue.data());
        //}
        //else
        //{
        //    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_WHITE);
        //    //drawList->AddText({0, 0}, IM_COL32_WHITE, aValue.data());
        //}
    }
    else
    {
        uint endCol = Serialization::KaraokeDocument::Get().GetEndColor();
        endCol = IM_COL32_FROM_DOC(endCol) | (aUseAlpha ? 0 : 0xFF000000);
        //ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_FROM_DOC(endCol) | (aUseAlpha ? 0 : 0xFF000000));
        //drawList->AddText({0, 0}, IM_COL32_FROM_DOC(endCol) | (aUseAlpha ? 0 : 0xFF000000), aValue.data());
        drawList->AddText(GetFont(), size.y /*GetFont()->FontSize * GetFont()->Scale*/, pos, endCol, aValue.data());
    }
    //Text(aValue.data());
    //drawList->PopTextureID();
    Dummy(size);
    //ImGui::PopStyleColor();
    bool clicked = IsItemClicked(0);
    float triangleSize = DPI_SCALED(5);
    if(aShowProgress)
    {
        ImDrawList* drawList = GetWindowDrawList();
        if(aValue.empty() || aValue == " ")
        {
            drawList->AddTriangleFilled({size.x + timeStartPos.x - 1, timeStartPos.y + 1}, {size.x + timeStartPos.x - triangleSize, timeStartPos.y + triangleSize}, {size.x + timeStartPos.x - 1, timeStartPos.y + triangleSize}, IM_COL32(255, 200, 255, 255));
        }
        else
        {
            drawList->AddTriangleFilled(timeStartPos, {timeStartPos.x + triangleSize, timeStartPos.y + triangleSize}, {timeStartPos.x, timeStartPos.y + triangleSize}, IM_COL32_WHITE);
            drawList->AddLine(timeStartPos, timeEndPos, IM_COL32_WHITE, DPI_SCALED(2));
            //drawList->AddLine({0, 0}, size, IM_COL32_WHITE, DPI_SCALED(2));
        }
    }
    //ImDrawData drawData = {};
    //drawData.Valid = true;
    //drawData.AddDrawList(drawList);
    //drawData.DisplayPos = {};
    //drawData.DisplaySize = size;
    //
    //static GLint temp_texture;
    //GLint last_texture;
    //GLint last_active_texture;
    //
	//glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    //glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
    //glActiveTexture(GL_TEXTURE0);
    ////if(temp_texture == 0)
    //{
	//    glGenTextures(1, &temp_texture);
    //}
	//glBindTexture(GL_TEXTURE_2D, temp_texture);
    ////if(temp_texture == 0)
    //{
	//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
	    //glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    //}
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, timeEndPos.y - pos.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	//glClearColor(1, 1, 1, 1);
	//glClear(GL_COLOR_BUFFER_BIT);
    //MainWindow_RenderCustomDrawData(&drawData, size.x, timeEndPos.y - pos.y);
    //glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, size.x, timeEndPos.y - pos.y, 0);
	//glBindTexture(GL_TEXTURE_2D, last_texture);
    //glActiveTexture(last_active_texture);
    //GetWindowDrawList()->AddImage((ImTextureID)temp_texture, pos, {size.x, timeEndPos.y - pos.y});
    //drawList->_ClearFreeMemory();
    return clicked;
}

void ImGui::Ext::SetColor(unsigned int aCol)
{
    PushStyleColor(ImGuiCol_Text, aCol);
}

void ImGui::Ext::ClearColor()
{
    PopStyleColor(GetCurrentContext()->ColorStack.size());
}

bool ImGui::Ext::ToggleSwitch(const char *aLabel, bool *aValue)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(aLabel);
    const ImVec2 label_size = CalcTextSize(aLabel, NULL, true);

    const float square_sz = GetFrameHeight();
    const ImVec2 size = ImVec2(square_sz * 2.5f, square_sz);
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, pos + ImVec2(size.x + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *aValue = !(*aValue);
        MarkItemEdited(id);
    }

    const ImRect check_bb(pos, pos + size);
    RenderNavHighlight(total_bb, id);
    RenderFrame(check_bb.Min + style.ItemInnerSpacing, check_bb.Max - style.ItemInnerSpacing, GetColorU32(*aValue ? ImGuiCol_CheckMark : ImGuiCol_FrameBg), true, style.FrameRounding);
    ImU32 check_col = GetColorU32(ImGuiCol_CheckMark);
    bool mixed_value = (g.LastItemData.InFlags & ImGuiItemFlags_MixedValue) != 0;
    ImRect pad;
    if (mixed_value)
    {
        // Undocumented tristate/mixed/indeterminate checkbox (#2644)
        // This may seem awkwardly designed because the aim is to make ImGuiItemFlags_MixedValue supported by all widgets (not just checkbox)
        pad = ImRect((size.x - square_sz) * .5f, 0, (size.x - square_sz) * .5f, 0);
        //window->DrawList->AddCircleFilled(check_bb.GetCenter(), square_sz * .5f, GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
    }
    else if (*aValue)
    {
        //const float pad = ImMax(1.0f, IM_TRUNC(square_sz / 6.0f));
        //RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad * 2.0f);
        pad = ImRect(size.x - square_sz, 0, 0, 0);
    }
    else
    {
        pad = ImRect(0, 0, size.x - square_sz, 0);
    }
    window->DrawList->AddRectFilled(check_bb.Min + pad.Min, check_bb.Max - pad.Max, GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button), style.FrameRounding);

    ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
    if (g.LogEnabled)
        LogRenderedText(&label_pos, mixed_value ? "[~]" : *aValue ? "[x]" : "[ ]");
    if (label_size.x > 0.0f)
        RenderText(label_pos, aLabel);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, aLabel, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*aValue ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}

bool ImGui::Ext::StepInt(const char *aLabel, int& aValue, int aSmallStep, int aLargeStep)
{
    bool output = false;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + DPI_SCALED(5));
    ImGui::Text(aLabel);
    ImGui::SameLine();
    ImGui::PushID(aLabel);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - DPI_SCALED(5));
    if(ImGui::Button("<<", {DPI_SCALED(40), 0}))
    {
        output = true;
        aValue -= aLargeStep;
    }
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - DPI_SCALED(5));
    if(ImGui::Button("<", {DPI_SCALED(40), 0}))
    {
        output = true;
        aValue -= aSmallStep;
    }
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - DPI_SCALED(5));
    ImGui::SetNextItemWidth(DPI_SCALED(60));
    if(ImGui::DragInt("##StepInt", &aValue))
    {
        output = true;
    }
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - DPI_SCALED(5));
    if(ImGui::Button(">", {DPI_SCALED(40), 0}))
    {
        output = true;
        aValue += aSmallStep;
    }
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - DPI_SCALED(5));
    if(ImGui::Button(">>", {DPI_SCALED(40), 0}))
    {
        output = true;
        aValue += aLargeStep;
    }
    ImGui::PopID();
    return output;
}

ImVector<ImVec2> ImGui::Ext::TextWrappedWithOverdraw(const char *fmt, va_list args)
{
    return ImVector<ImVec2>();
}

bool ImGui::Ext::TabMenu(ImVector<std::string> someLabels, int *aValue)
{
    bool output = false;
    for(int i = 0; i < someLabels.size(); i++)
    {
        bool isSelected = *aValue == i;
        if(isSelected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, GetColorU32(ImGuiCol_ButtonActive));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GetColorU32(ImGuiCol_ButtonActive));
        }
        if(ImGui::Button(someLabels[i].data()))
        {
            output = true;
            *aValue = i;
        }
        if(isSelected)
        {
            ImGui::PopStyleColor(2);
        }
    }
}
