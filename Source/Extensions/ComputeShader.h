//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#pragma once
#include "Windows/MainWindow.h"
#include <unordered_map>
#include "ShaderBuffer.h"

typedef int unsigned uint;
class ComputeShader
{
    struct Reflection
    {
    public:
        struct BufferInfo
        {
        public:
            uint Group;
            uint Binding;
            WGPUBufferBindingType BindType;
            WGPUBindGroupLayoutEntry AsLayoutEntry() const;
            WGPUBindGroupEntry AsBindGroupEntry(ShaderBuffer aBuffer) const;
        };
        struct KernelInfo
        {
        public:
            int SizeX;
            int SizeY;
            int SizeZ;
        };
        std::unordered_map<std::string, BufferInfo> Buffers;
        std::unordered_map<std::string, KernelInfo> Kernels;    
    public:
        std::vector<WGPUBindGroupLayout> AsLayouts();
        std::vector<WGPUBindGroup> AsBindGroups(std::unordered_map<std::string, ShaderBuffer>& someBuffers);
    };
public:
    static ComputeShader Compile(std::string someCode);
    static ComputeShader Load(std::string aFilePath);
    void Dispatch(std::string aKernelName, int aSizeX, int aSizeY = 1, int aSizeZ = 1);

    void SetBool(std::string aName, bool aValue);
    void SetInt(std::string aName, int aValue);
    void SetUInt(std::string aName, uint aValue);
    void SetFloat(std::string aName, float aValue);
    void SetTexture();
    void SetBuffer(std::string aName, ShaderBuffer aBuffer);
    void GetTexture();
    ShaderBuffer GetBuffer(std::string aName);

private:
    ComputeShader();
    void Initialize(std::string someCode);

    std::string myCode;
    Reflection myReflection;
    WGPUPipelineLayout myLayout;
    WGPUShaderModule myShader;
    std::unordered_map<std::string, double> myConstOverrides;
    std::unordered_map<std::string, ShaderBuffer> myBuffers;
};