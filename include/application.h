#ifndef APPLICATION_H
#define APPLICATION_H

#include <webgpu/webgpu.hpp>
#include <glfw/glfw3.h>
#include <glfw3webgpu/glfw3webgpu.h>
#include <structs.h>
#include <resourceManager.h>
#include <pipelineData.h>
#include <glm/gtc/matrix_transform.hpp>
#include <physicmanager.h>

class Application {
public:

    explicit Application(std::vector<Object> &vData);

// A function called only once at the beginning. Returns false is init failed.
    bool onInit(bool fullScreen);

    // A function called at each frame, guaranteed never to be called before `onInit`.
    void onFrame();

    // A function called only once at the very end.
    void onFinish();

    void initSimulationStruct(VectorXR &x, VectorXR &v, VectorXR &f, float timeStep);

    void initConstraintsStruct(ConstraintType constraint, std::vector<Vector32i> &id, std::vector<VectorXR> &data);

    void computeSimulation(ComputeOperation compute_operation);

    void computeStretch(int color_count);

    void readP(VectorXR &p);

    bool isRunning() { return !glfwWindowShouldClose(m_window); }

    void onResize();

    //Buffers
    wgpu::Buffer m_indexBuffer = nullptr;
    wgpu::Buffer m_vertexBuffer = nullptr;
    wgpu::Buffer m_normalBuffer = nullptr;
    wgpu::Buffer m_uTimeBuffer = nullptr;
    wgpu::Buffer m_mvpBuffer = nullptr;

    //Compute Buffers
    wgpu::Buffer m_xBuffer = nullptr;
    wgpu::Buffer m_piBuffer = nullptr;
    wgpu::Buffer m_pfBuffer = nullptr;
    wgpu::Buffer m_vBuffer = nullptr;
    wgpu::Buffer m_fBuffer = nullptr;
    wgpu::Buffer m_stepDataBuffer = nullptr;
    wgpu::Buffer m_mapBuffer = nullptr;
    std::vector<wgpu::Buffer> m_stenCountBuffer;
    std::vector<wgpu::Buffer> m_dataBuffer;
    std::vector<wgpu::Buffer> m_idxBuffer;

    //Compute structs (bindings & pipelines)
    SimulationStepStruct m_computeP_BindPipe;
    SimulationStepStruct m_computeV_BindPipe;
    std::vector<SimulationStepStruct> m_stretchConstraint_BindPipe;

    //Compute Data
    std::vector<int> m_idxSizes;
    std::vector<int> m_dataSizes;
    int m_numDof{};

    std::vector<Object> &m_vertexData;
    int m_idxCount{};
    MyUniforms m_mvpUniforms{};
    float deltaTime = 0;
    float lastFrameT = 0;

    //PhysicManager
//    PhysicManager &physicManager;
//    PhysicManagerPBD &physicManagerPbd;

private:
    const size_t BYTES_PER_ROW_ALIGNMENT = 256;
    // Everything that is initialized in `onInit` and needed in `onFrame`.
    wgpu::Instance m_instance = nullptr;
    wgpu::Surface m_surface = nullptr;
    GLFWwindow *m_window = nullptr;
    wgpu::Adapter m_adapter = nullptr;
    wgpu::Device m_device = nullptr;
    wgpu::Limits m_deviceLimits;
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

    //GPU compute values
    wgpu::ComputePassEncoder m_computePass = nullptr;
//    wgpu::ComputePipeline m_computePipeline = nullptr;
    wgpu::ShaderModule m_computeShaderModule = nullptr;
    std::vector<wgpu::BindGroupLayoutEntry> m_computeBindingLayoutEntries;
    wgpu::BindGroupLayout m_computeBindGroupLayout = nullptr;
//    wgpu::BindGroup m_computeBindGroup = nullptr;
    wgpu::PipelineLayout m_computePipelineLayout = nullptr;

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

    wgpu::BindGroup createConstraintBindings(size_t idx_size, size_t data_size, int color);

    wgpu::BindGroup createSimulationBindings(ComputeOperation compute_operation);

    wgpu::ComputePipeline createComputePipeline(ComputeOperation shader);

    uint32_t respectAlignment(uint32_t size);
};

#endif