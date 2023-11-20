#ifndef APPLICATION_H
#define APPLICATION_H

#include <webgpu/webgpu.hpp>
#include <glfw/glfw3.h>
#include <glfw3webgpu/glfw3webgpu.h>
#include <structs.h>
#include <resourceManager.h>
#include <pipelineData.h>
#include <glm/gtc/matrix_transform.hpp>

class Application {
public:

    Application();

// A function called only once at the beginning. Returns false is init failed.
    bool onInit(bool fullScreen);

    // A function called at each frame, guaranteed never to be called before `onInit`.
    void onFrame();

    // A function called only once at the very end.
    void onFinish();

    bool isRunning() { return !glfwWindowShouldClose(m_window); }

    void onResize();

    //Buffers
    wgpu::Buffer m_vertexBuffer = nullptr;
    wgpu::Buffer m_normalBuffer = nullptr;
    wgpu::Buffer m_uTimeBuffer = nullptr;
    wgpu::Buffer m_mvpBuffer = nullptr;

    std::vector<Object> m_vertexData;
    int m_idxCount{};
    MyUniforms m_mvpUniforms{};
    float deltaTime = 0;
    float lastFrameT = 0;

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
    wgpu::TextureFormat m_SwapChainFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat m_depthBufferFormat = wgpu::TextureFormat::Undefined;
    wgpu::Texture m_depthBuffer = nullptr;
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

    //Camera view variables
    float lastX = 0;
    float lastY = 0;
    float sensitivity = 0.1f;
    float yaw = -90.f;
    float pitch = 0;
    float camSpeed = 10.f;
    CameraState m_camState{};

    bool initWindowAndDevice(int width, int height);

    void initSwapChain();

    void createPipeline();

    void initDepthBuffer();

    void initBuffers();

    void initBindings();

    void onMouseMove(double x, double y);

    void onKeyPressed(int key, int action);
};

#endif