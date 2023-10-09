#define WEBGPU_CPP_IMPLEMENTATION

#include "webgpu/webgpu.hpp"

#ifndef WGPU_PS_PIPELINEDATA_H
#define WGPU_PS_PIPELINEDATA_H

using namespace wgpu;
class PipelineData{
public:
    RenderPipelineDescriptor pipeDesc;
    VertexBufferLayout vertexBufferLayout;
    std::vector<VertexAttribute> attributes;
    FragmentState fragmentState;
    BlendState blendState;
    ColorTargetState colorTarget;

    PipelineData();
    void setVertexDescription(ShaderModule shaderModule, int attribCount);
    void setPrimitiveDescriptor();
    void setFragmentDescriptor(WGPUTextureFormat swapChainFormat, ShaderModule shaderModule);
    void setMisc();
};


#endif //WGPU_PS_PIPELINEDATA_H
