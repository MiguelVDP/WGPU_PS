#include <webgpu/webgpu.hpp>

#ifndef WGPU_PS_PIPELINEDATA_H
#define WGPU_PS_PIPELINEDATA_H

class PipelineData{
public:
    wgpu::RenderPipelineDescriptor pipeDesc;
    wgpu::VertexBufferLayout vertexBufferLayout;
    std::vector<wgpu::VertexAttribute> attributes;
    wgpu::FragmentState fragmentState;
    wgpu::BlendState blendState;
    wgpu::ColorTargetState colorTarget;

    PipelineData();
    void setVertexDescription(wgpu::ShaderModule shaderModule, int attribCount);
    void setPrimitiveDescriptor();
    void setFragmentDescriptor(WGPUTextureFormat swapChainFormat, wgpu::ShaderModule shaderModule);
    void setMisc();
};


#endif //WGPU_PS_PIPELINEDATA_H
