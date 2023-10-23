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
    wgpu::DepthStencilState depthStencilState;

    PipelineData();
    void setVertexDescription(wgpu::ShaderModule shaderModule);
    void setPrimitiveDescriptor();
    void setFragmentDescriptor(wgpu::TextureFormat swapChainFormat, wgpu::ShaderModule shaderModule);
    void setDepthStencilDescriptor(wgpu::TextureFormat depthTextureFormat);
    void setMisc();
};


#endif //WGPU_PS_PIPELINEDATA_H
