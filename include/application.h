#pragma once

#include <webgpu/webgpu.hpp>
#include <glfw/glfw3.h>
#include <glfw3webgpu/glfw3webgpu.h>
#include <structs.h>
#include <resourceManager.h>
#include <pipelineData.h>

class Application {
public:
    // A function called only once at the beginning. Returns false is init failed.
    bool onInit(int width, int height);

    // A function called at each frame, guaranteed never to be called before `onInit`.
    void onFrame();

    // A function called only once at the very end.
    void onFinish();

    void set_MVPUniforms(MyUniforms u) { m_mvpUniforms = u; }

    //Buffers
    wgpu::Buffer m_vertexBuffer;
    wgpu::Buffer m_uTimeBuffer;
    wgpu::Buffer m_mvpBuffer;

    std::vector<VertexAttributes> m_vertexData;
    int m_idxCount;
    MyUniforms m_mvpUniforms;

private:
    // Everything that is initialized in `onInit` and needed in `onFrame`.
    wgpu::Instance m_instance = nullptr;
    wgpu::Surface m_surface = nullptr;
    GLFWwindow *m_window = nullptr;
    wgpu::Adapter m_adapter = nullptr;
    wgpu::Device m_device = nullptr;
    std::unique_ptr<wgpu::ErrorCallback> m_errorCallbackHandle;
    wgpu::Queue m_queue = nullptr;
    wgpu::SwapChain m_swapChain = nullptr;
    wgpu::ShaderModule m_shaderModule = nullptr;
    wgpu::TextureFormat m_SwapChainFormat;
    wgpu::TextureFormat m_depthTextureFormat;
    wgpu::Texture m_depthTexture = nullptr;
    PipelineData m_pipelineData;
    wgpu::RenderPipeline m_renderPipeline = nullptr;
    std::vector<wgpu::BindGroupLayoutEntry> m_bindingLayoutEntries;
    wgpu::BindGroupLayout m_bindGroupLayout = nullptr;
    wgpu::BindGroup m_bindGroup = nullptr;
    wgpu::PipelineLayout m_pipelineLayout = nullptr;

    wgpu::TextureView m_nextTexture = nullptr;
    wgpu::CommandEncoder m_encoder = nullptr;
    wgpu::RenderPassEncoder m_renderPass = nullptr;
    wgpu::TextureView m_depthTextureView = nullptr;

    bool initInstanceAdapter();

    void initDeviceLimits(int width, int height);

    void initSwapChain(int width, int height);

    void createPipeline();

    void createDepthTexture();

    void initBuffers();

    void initBindings();
};
