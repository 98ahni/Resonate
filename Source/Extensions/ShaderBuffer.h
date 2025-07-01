//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#pragma once
#include "Windows/MainWindow.h"
#include <vector>
#include <tuple>

typedef int unsigned uint;
namespace emscripten { class val; }
class ShaderBuffer
{
    enum State { Invalid, Valid, Dispatched, Dirty };
public:
    static ShaderBuffer Create();
    template<typename T> static ShaderBuffer Create(const std::vector<T>& someData)
        { ShaderBuffer output = Create(); output.SetData(someData); return output; }
    ShaderBuffer(const ShaderBuffer& aCopy);
    ~ShaderBuffer();
    template<typename T> void SetData(const std::vector<T>& someData) { SetDataInternal(someData.data(), someData.size() * sizeof(T)); }
    template<typename T> std::vector<T> GetData()
    {
        auto[data, size] = GetDataInternal();
        if(size == 0 || size % sizeof(T)) return std::vector<T>();
        std::vector<T> output(size / sizeof(T));
        memcpy(output.data(), data, size);
        return output;
    }

    WGPUBuffer Dispatch(WGPUBufferUsage aUsage = WGPUBufferUsage_None);
    size_t ByteSize() const;
    template<typename T> size_t Size() { return ByteSize() % sizeof(T) ? ByteSize() / sizeof(T) : 0; }
    void Resize(size_t someNumBytes);

private:
    ShaderBuffer(); // Explicitly disallow constructor to force usage of Create<T>().

    void SetDataInternal(void* someData, size_t aSize);
    std::tuple<void*, size_t> GetDataInternal();
    typedef void (*DataReadyCallback)(void* someData, size_t aSize, void* someUserData);
    void GetDataAsyncInternal(DataReadyCallback aCallback, void* someUserData);
    void Invalidate();
    
    void SetDataJavaScript(emscripten::val anArray);
    void GetDataAsyncJavaScript(emscripten::val aCallbackFunc);

    struct BufferData
    {
        public:
        uint RefCount = 0;
        State DataState;
        WGPUBuffer Buffer;
        WGPUBuffer ReadBackBuffer;
        WGPUBufferUsage Usage;
        void* Raw;
        size_t ByteSize;
    };
    BufferData* myData;
};