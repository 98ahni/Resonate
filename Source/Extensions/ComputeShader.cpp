//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include "ComputeShader.h"
#include <emscripten/bind.h>
#include <StringTools.h>
#include <fstream>
#include <cmath>
#include <Defines.h>

EMSCRIPTEN_BINDINGS(ComputeShaderModule)
{
    emscripten::class_<ComputeShader>("ComputeShader")
        .class_function("Compile", &ComputeShader::Compile)
        .class_function("Load", &ComputeShader::Load)
        .function("Dispatch", &ComputeShader::Dispatch)
        .function("SetBool", &ComputeShader::SetBool)
        .function("SetInt", &ComputeShader::SetInt)
        .function("SetUInt", &ComputeShader::SetUInt)
        .function("SetFloat", &ComputeShader::SetFloat)
        .function("SetBuffer", &ComputeShader::SetBuffer)
        .function("GetBuffer", &ComputeShader::GetBuffer)
        .function("SetTexture", &ComputeShader::SetTexture)
        .function("GetTexture", &ComputeShader::GetTexture);
}

WGPUBindGroupLayoutEntry ComputeShader::Reflection::BufferInfo::AsLayoutEntry() const
{
    return (WGPUBindGroupLayoutEntry){
        .binding = Binding,
        .visibility = WGPUShaderStage_Compute,
        .buffer = (WGPUBufferBindingLayout) {
            .type           = BindType
        }
    };
}

WGPUBindGroupEntry ComputeShader::Reflection::BufferInfo::AsBindGroupEntry(ShaderBuffer aBuffer) const
{
    return (WGPUBindGroupEntry){
        .binding = Binding,
        .buffer = aBuffer.Dispatch((WGPUBufferUsage)(WGPUBufferUsage_CopyDst | (
            BindType == WGPUBufferBindingType_Uniform ? WGPUBufferUsage_Uniform : (
                BindType == WGPUBufferBindingType_ReadOnlyStorage ? WGPUBufferUsage_Storage : (
                    BindType == WGPUBufferBindingType_Storage ? (WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc) : 
                        WGPUBufferUsage_None))))),
        .offset = 0,
        .size = aBuffer.ByteSize()
    };
}

std::vector<WGPUBindGroupLayout> ComputeShader::Reflection::AsLayouts()
{
    std::vector<std::vector<WGPUBindGroupLayoutEntry>> entries = {};
    for(const auto& [name, info] : Buffers)
    {
        while(entries.size() <= info.Group)
        {
            entries.push_back(std::vector<WGPUBindGroupLayoutEntry>());
        }
        entries[info.Group].push_back(info.AsLayoutEntry());
    }
    std::vector<WGPUBindGroupLayout> output = {};
    for(int i = 0; i < entries.size(); i++)
    {
        WGPUBindGroupLayoutDescriptor desc = {
            .label      = "Compute - Bind group layout",
            .entryCount = entries[i].size(),
            .entries    = entries[i].data(),
        };
        output.push_back(wgpuDeviceCreateBindGroupLayout(MainWindow::Device, &desc));
    }
    return output;
}

std::vector<WGPUBindGroup> ComputeShader::Reflection::AsBindGroups(std::unordered_map<std::string, ShaderBuffer>& someBuffers)
{
    std::vector<std::vector<WGPUBindGroupEntry>> entries = {};
    std::vector<std::vector<WGPUBindGroupLayoutEntry>> layoutEntries = {};
    for(const auto& [name, info] : Buffers)
    {
        while(entries.size() <= info.Group)
        {
            entries.push_back(std::vector<WGPUBindGroupEntry>());
            layoutEntries.push_back(std::vector<WGPUBindGroupLayoutEntry>());
        }
        entries[info.Group].push_back(info.AsBindGroupEntry(someBuffers.at(name)));
        layoutEntries[info.Group].push_back(info.AsLayoutEntry());
    }
    std::vector<WGPUBindGroupLayout> layout = {};
    for(int i = 0; i < layoutEntries.size(); i++)
    {
        WGPUBindGroupLayoutDescriptor desc = {
            .label      = "Compute - Bind group layout",
            .entryCount = layoutEntries[i].size(),
            .entries    = layoutEntries[i].data(),
        };
        layout.push_back(wgpuDeviceCreateBindGroupLayout(MainWindow::Device, &desc));
    }
    std::vector<WGPUBindGroup> output = {};
    for(int i = 0; i < entries.size(); i++)
    {
        WGPUBindGroupDescriptor desc = {
            .label      = "Compute - Bind group",
            .layout     = layout[i],
            .entryCount = entries[i].size(),
            .entries    = entries[i].data(),
        };
        output.push_back(wgpuDeviceCreateBindGroup(MainWindow::Device, &desc));
    }
    return output;
}

ComputeShader::ComputeShader()
{
}

ComputeShader ComputeShader::Compile(std::string someCode)
{
    ComputeShader compute = ComputeShader();
    compute.Initialize(someCode);
    return compute;
}

ComputeShader ComputeShader::Load(std::string aFilePath)
{
    ComputeShader compute = ComputeShader();
    std::ifstream docFile(aFilePath.c_str());
    std::string line;
    std::string code = "";
    while(std::getline(docFile, line))
    {
        code += line + "\n";
    }
    docFile.close();
    compute.Initialize(code);
    return compute;
}

void CompilationCallback(WGPUCompilationInfoRequestStatus status, const WGPUCompilationInfo* compilationInfo, void* data)
{
    if(status == WGPUCompilationInfoRequestStatus_Success) DBGprintf("WGPUCompilationInfoRequestStatus_Success\n");
    if(status == WGPUCompilationInfoRequestStatus_Error) printf("WGPUCompilationInfoRequestStatus_Error\n");
    if(status == WGPUCompilationInfoRequestStatus_DeviceLost) printf("WGPUCompilationInfoRequestStatus_DeviceLost\n");
    if(status == WGPUCompilationInfoRequestStatus_Unknown) printf("WGPUCompilationInfoRequestStatus_Unknown\n");
    for(int i = 0; i < compilationInfo->messageCount; i++)
    {
        if(compilationInfo->messages[i].message)
            printf("%s\n", compilationInfo->messages[i].message);
    }
}

void ComputeShader::Initialize(std::string someCode)
{
    myCode = someCode;
    std::regex bufferEx = std::regex("@group\\s*\\(\\s*(\\d+)\\s*\\)\\s*@binding\\s*\\(\\s*(\\d+)\\s*\\)\\s*var\\s*\\<\\s*(\\w+)\\s*(?:,\\s*(\\w+)\\s*)?>\\s*(\\w+)");
    std::vector<std::string> bufferDecl = StringTools::Split(myCode, bufferEx, true);
    for(std::string decl : bufferDecl)
    {
        std::smatch bufferInfo;
        if(!std::regex_search(decl, bufferInfo, bufferEx)) { continue; }
        // [0] is the full match
        Reflection::BufferInfo info = {
            .Group = (uint)std::stoi(bufferInfo[1]),
            .Binding = (uint)std::stoi(bufferInfo[2]),
            .BindType = 
                bufferInfo[3] == "storage" ? (
                    bufferInfo.size() == 6 && bufferInfo[4] == "read_write" ? 
                        WGPUBufferBindingType_Storage : WGPUBufferBindingType_ReadOnlyStorage) : (
                bufferInfo[3] == "uniform" ? WGPUBufferBindingType_Uniform : 
                WGPUBufferBindingType_Undefined)
        };
        
        std::string varName = bufferInfo[bufferInfo.size() == 6 ? 5 : 4];
        myReflection.Buffers[varName] = info;
    }

    std::regex kernelEx = std::regex("@compute\\s*@workgroup_size\\s*\\(\\s*([\\d,\\s]+)\\s*\\)\\s*fn\\s*(\\w+)");
    std::regex sizeEx = std::regex("\\d+");
    std::vector<std::string> kernelDecl = StringTools::Split(myCode, kernelEx, true);
    for(std::string decl : kernelDecl)
    {
        std::smatch kernelInfo;
        if(!std::regex_search(decl, kernelInfo, kernelEx)) { continue; }
        // [0] is the full match
        std::string size = kernelInfo[1];   // might be (###) or (#, #, #)
        std::vector<std::string> sizes = StringTools::Split(size, sizeEx, true);
        std::string fnName = kernelInfo[2];
        
        myReflection.Kernels[fnName] = {
            .SizeX = sizes.size() > 0 ? std::stoi(sizes[0]) : 1,
            .SizeY = sizes.size() > 1 ? std::stoi(sizes[1]) : 1,
            .SizeZ = sizes.size() > 2 ? std::stoi(sizes[2]) : 1
        };
    }
    WGPUShaderModuleWGSLDescriptor wgslDesc{
        .chain  = {
            .sType = WGPUSType_ShaderModuleWGSLDescriptor,
        },
        .code = myCode.c_str(),
    };
    WGPUShaderModuleDescriptor moduleDesc{
        .nextInChain = &wgslDesc.chain
    };
    myShader = wgpuDeviceCreateShaderModule(MainWindow::Device, &moduleDesc);
    wgpuShaderModuleGetCompilationInfo(myShader, CompilationCallback, myCode.data());

    std::vector<WGPUBindGroupLayout> layouts = myReflection.AsLayouts();
    WGPUPipelineLayoutDescriptor layoutDesc{
        .label                = "Compute - Pipeline layout",
        .bindGroupLayoutCount = layouts.size(),
        .bindGroupLayouts     = layouts.data()
    };
    myLayout = wgpuDeviceCreatePipelineLayout(MainWindow::Device, &layoutDesc);
}

void ComputeShader::Dispatch(std::string aKernelName, int aSizeX, int aSizeY, int aSizeZ)
{
    std::vector<WGPUConstantEntry> constants = {};
    for(const auto& [name, val] : myConstOverrides)
    {
        constants.push_back((WGPUConstantEntry){ .key = name.c_str(), .value = val });
    }
    WGPUComputePipelineDescriptor pipeDesc{
        .label   = "Compute - Compute pipeline",
        .layout  = myLayout,
        .compute = (WGPUProgrammableStageDescriptor){
            .module        = myShader,
            .entryPoint    = aKernelName.c_str(),
            .constantCount = constants.size(),
            .constants     = constants.data(),
        }
    };
    WGPUComputePipeline pipeline = wgpuDeviceCreateComputePipeline(MainWindow::Device, &pipeDesc);
    
    std::vector<WGPUBindGroup> groups = myReflection.AsBindGroups(myBuffers);
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(MainWindow::Device, NULL);
    WGPUComputePassEncoder compute = wgpuCommandEncoderBeginComputePass(encoder, NULL);
    wgpuComputePassEncoderSetPipeline(compute, pipeline);
    for(int group = 0; group < groups.size(); group++)
    {
        wgpuComputePassEncoderSetBindGroup(compute, group, groups[group], 0, NULL);
    }
    Reflection::KernelInfo& kInfo = myReflection.Kernels.at(aKernelName);
    wgpuComputePassEncoderDispatchWorkgroups(compute, std::ceil(aSizeX / (float)kInfo.SizeX), std::ceil(aSizeY / (float)kInfo.SizeY), std::ceil(aSizeZ / (float)kInfo.SizeZ));
    wgpuComputePassEncoderEnd(compute);
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, {});
    wgpuQueueSubmit(wgpuDeviceGetQueue(MainWindow::Device), 1, &command);
}

void ComputeShader::SetBool(std::string aName, bool aValue)
{
    myConstOverrides[aName] = aValue;
}

void ComputeShader::SetInt(std::string aName, int aValue)
{
    myConstOverrides[aName] = aValue;
}

void ComputeShader::SetUInt(std::string aName, uint aValue)
{
    myConstOverrides[aName] = aValue;
}

void ComputeShader::SetFloat(std::string aName, float aValue)
{
    myConstOverrides[aName] = aValue;
}

void ComputeShader::SetTexture()
{
}

void ComputeShader::SetBuffer(std::string aName, ShaderBuffer aBuffer)
{
    myBuffers.insert_or_assign(aName, aBuffer);
}

void ComputeShader::GetTexture()
{
}

ShaderBuffer ComputeShader::GetBuffer(std::string aName)
{
    return myBuffers.at(aName);
}
