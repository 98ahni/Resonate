//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include <emscripten.h>
#include <emscripten/bind.h>
#define private public
#include "ShaderBuffer.h"
#include <Defines.h>
#undef private

ShaderBuffer ShaderBuffer_Create()
{
    return ShaderBuffer::Create();
}

void ShaderBuffer::SetDataJavaScript(emscripten::val anArray)
{
    unsigned int length = anArray["length"].as<unsigned int>();
    unsigned int byteSize = anArray["BYTES_PER_ELEMENT"].as<unsigned int>() * length;
    if(byteSize != myData->ByteSize)
    {
        Invalidate();
        myData->ByteSize = byteSize;
        if(myData->Raw) { free(myData->Raw); }
        myData->Raw = malloc(byteSize);
    }
    auto memory = emscripten::val::module_property("HEAPU8")["buffer"];
    auto memoryView = anArray["constructor"].new_(memory, reinterpret_cast<uintptr_t>(myData->Raw), length);
    memoryView.call<void>("set", anArray);
    if(myData->DataState != Invalid)
    {
        myData->DataState = Dirty;
    }
}

void ShaderBuffer::GetDataAsyncJavaScript(emscripten::val aCallbackFunc)
{
    if(!emscripten::val::global("Module").hasOwnProperty("ShaderBufferGetDataCallbacks"))
    {
        emscripten::val::global("Module").set("ShaderBufferGetDataCallbacks", emscripten::val::global("Map").new_());
    }
    auto callbacks = emscripten::val::global("Module")["ShaderBufferGetDataCallbacks"];
    int key = (int)aCallbackFunc.as_handle();
    while(callbacks.call<bool>("has", key)) { key++; }
    callbacks.call<void>("set", key, aCallbackFunc);
    GetDataAsyncInternal([](void* data, size_t size, void* userdata){
        auto memory = emscripten::val::global("wasmMemory")["buffer"];
        auto arrBuff = memory.call<emscripten::val, emscripten::val, emscripten::val>("slice", emscripten::val((size_t)data), emscripten::val(size + (size_t)data));
        auto callbacks = emscripten::val::global("Module")["ShaderBufferGetDataCallbacks"];
        auto thisCallback = callbacks.call<emscripten::val>("get", (int)userdata);
        thisCallback(arrBuff);
        callbacks.call<void>("delete", (int)userdata);
    }, (void*)key);
}

EMSCRIPTEN_BINDINGS(ShaderBufferModule)
{
    emscripten::class_<ShaderBuffer>("ShaderBuffer")
        .class_function("Create", &ShaderBuffer_Create)
        .function("SetData", &ShaderBuffer::SetDataJavaScript)
        .function("GetDataAsync", &ShaderBuffer::GetDataAsyncJavaScript)
        .function("Resize", &ShaderBuffer::Resize)
        .property("byteSize", &ShaderBuffer::ByteSize);
}

ShaderBuffer::ShaderBuffer()
{
    myData = new BufferData();
    myData->DataState = Invalid;
    myData->RefCount = 1;
    myData->ReadBackBuffer = {0};
    myData->ByteSize = 0;
    myData->Raw = nullptr;
}

ShaderBuffer::ShaderBuffer(const ShaderBuffer& aCopy)
{
    if(!aCopy.myData)
    {
        myData = new BufferData();
        myData->DataState = Invalid;
        myData->RefCount = 0;
        myData->ReadBackBuffer = {0};
        myData->ByteSize = 0;
        myData->Raw = nullptr;
    }
    else
    {
        myData = aCopy.myData;
    }
    myData->RefCount++;
}

ShaderBuffer ShaderBuffer::Create()
{
    return ShaderBuffer();
}

ShaderBuffer::~ShaderBuffer()
{
    myData->RefCount--;
    if(myData->RefCount == 0)
    {
        Invalidate();
        if(myData->Raw) { free(myData->Raw); }
        delete myData;
        myData = nullptr;
    }
}

WGPUBuffer ShaderBuffer::Dispatch(WGPUBufferUsage aUsage)
{
    if(aUsage != WGPUBufferUsage_None && aUsage != myData->Usage) { Invalidate(); }
    if(myData->DataState == Invalid && myData->Raw)
    {
        myData->Usage = aUsage;
        WGPUBufferDescriptor buffDesc{ .size = myData->ByteSize, .usage = myData->Usage, .mappedAtCreation = false };
        myData->Buffer = wgpuDeviceCreateBuffer(MainWindow::Device, &buffDesc);
        myData->DataState = Dirty;
    }
    if(myData->DataState == Invalid) { return {0}; }
    if(myData->DataState == Dirty)
    {
        wgpuQueueWriteBuffer(wgpuDeviceGetQueue(MainWindow::Device), myData->Buffer, 0, myData->Raw, myData->ByteSize);
        myData->DataState = Valid;
    }
    myData->DataState = Dispatched;
    return myData->Buffer;
}

size_t ShaderBuffer::ByteSize() const
{
    return myData->ByteSize;
}

void ShaderBuffer::Resize(size_t someNumBytes)
{
    if(someNumBytes != myData->ByteSize)
    {
        Invalidate();
        void* oldPtr = myData->Raw;
        myData->Raw = malloc(someNumBytes);
        if(myData->Raw)
        {
            memcpy(myData->Raw, oldPtr, myData->ByteSize < someNumBytes ? myData->ByteSize : someNumBytes);
            free(oldPtr);
        }
        myData->ByteSize = someNumBytes;
    }
    if(myData->DataState != Invalid)
    {
        myData->DataState = Dirty;
    }
}

void ShaderBuffer::SetDataInternal(void *someData, size_t aSize)
{
    if(aSize != myData->ByteSize)
    {
        Invalidate();
        myData->ByteSize = aSize;
        if(myData->Raw) { free(myData->Raw); }
        myData->Raw = malloc(aSize);
    }
    memcpy(myData->Raw, someData, aSize);
    if(myData->DataState != Invalid)
    {
        myData->DataState = Dirty;
    }
}

std::tuple<void*, size_t> ShaderBuffer::GetDataInternal()
{
    if((myData->Usage & WGPUBufferUsage_CopySrc) == 0) { return {nullptr, 0}; }
    if(myData->DataState == Dispatched)
    {
        if(!myData->ReadBackBuffer)
        {
            WGPUBufferDescriptor buffDesc{
                .size = myData->ByteSize,
                .usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst,
                .mappedAtCreation = false
            };
            myData->ReadBackBuffer = wgpuDeviceCreateBuffer(MainWindow::Device, &buffDesc);
        }
        else
        {
            wgpuBufferUnmap(myData->ReadBackBuffer);
        }
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(MainWindow::Device, NULL);
        wgpuCommandEncoderCopyBufferToBuffer(encoder, myData->Buffer, 0, myData->ReadBackBuffer, 0, myData->ByteSize);
        WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, {});
        wgpuQueueSubmit(wgpuDeviceGetQueue(MainWindow::Device), 1, &command);
        bool isDone = false;
        wgpuBufferMapAsync(myData->ReadBackBuffer, WGPUMapMode_Read, 0, myData->ByteSize, [](WGPUBufferMapAsyncStatus status, void* done){ *(bool*)done = true; }, &isDone);
        while(!isDone) { emscripten_sleep(5); }
        memcpy(myData->Raw, wgpuBufferGetConstMappedRange(myData->ReadBackBuffer, 0, myData->ByteSize), myData->ByteSize);
        myData->DataState = Valid;
    }
    if(myData->DataState == Valid || myData->DataState == Dirty)
    {
        return {myData->Raw, myData->ByteSize};
    }
    return {nullptr, 0};
}

void ShaderBuffer::GetDataAsyncInternal(DataReadyCallback aCallback, void* someUserData)
{
    if((myData->Usage & WGPUBufferUsage_CopySrc) == 0) { aCallback(nullptr, 0, someUserData); }
    if(myData->DataState == Dispatched)
    {
        if(!myData->ReadBackBuffer)
        {
            WGPUBufferDescriptor buffDesc{
                .size = myData->ByteSize,
                .usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst,
                .mappedAtCreation = false
            };
            myData->ReadBackBuffer = wgpuDeviceCreateBuffer(MainWindow::Device, &buffDesc);
        }
        else
        {
            wgpuBufferUnmap(myData->ReadBackBuffer);
        }
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(MainWindow::Device, NULL);
        wgpuCommandEncoderCopyBufferToBuffer(encoder, myData->Buffer, 0, myData->ReadBackBuffer, 0, myData->ByteSize);
        WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, {});
        wgpuQueueSubmit(wgpuDeviceGetQueue(MainWindow::Device), 1, &command);
        std::tuple<BufferData*, DataReadyCallback, void*>* callbackData = new std::tuple<BufferData*, DataReadyCallback, void*>(myData, aCallback, someUserData);
        wgpuBufferMapAsync(myData->ReadBackBuffer, WGPUMapMode_Read, 0, myData->ByteSize, [](WGPUBufferMapAsyncStatus status, void* userdata){
            auto[data, callback, usrfwd] = *(std::tuple<BufferData*, DataReadyCallback, void*>*)userdata;
            memcpy(data->Raw, wgpuBufferGetConstMappedRange(data->ReadBackBuffer, 0, data->ByteSize), data->ByteSize);
            data->DataState = Valid;
            callback(data->Raw, data->ByteSize, usrfwd);
            delete userdata;
        }, callbackData);
    }
    else if(myData->DataState == Valid || myData->DataState == Dirty)
    {
        aCallback(myData->Raw, myData->ByteSize, someUserData);
    }
    else { aCallback(nullptr, 0, someUserData); }
}

void ShaderBuffer::Invalidate()
{
    myData->DataState = Invalid;
    if(myData->Buffer) wgpuBufferUnmap(myData->Buffer);
    if(myData->ReadBackBuffer) wgpuBufferUnmap(myData->ReadBackBuffer);
    myData->Buffer = {0};
    myData->ReadBackBuffer = {0};
}
