#include "application.h"
#include <executionTimer.h>

using namespace wgpu;

bool Application::onInit(bool fullScreen) {
    if (!glfwInit()) {  // initialize GLFW & check for any GLFW error
        std::cout << "Could not initialize GLFW!" << std::endl;
        return false;
    }

    if (fullScreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        if (!initWindowAndDevice(mode->width, mode->height)) return false;
    } else {
        if (!initWindowAndDevice(640, 480)) return false;
    }


    m_queue = m_device.getQueue();

    initSwapChain();

    initDepthBuffer();

    initBuffers();

    initBindings();

    createPipeline();

    m_idxCount = static_cast<int>(m_vertexData[0].triangles.size());

    if (!m_window) {  //Check for errors
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return false;
    }

    return true;
}

void Application::onFrame() {
    auto currentFrameT = (float) glfwGetTime();
    deltaTime = currentFrameT - lastFrameT;
    lastFrameT = currentFrameT;

    glfwPollEvents();
    m_nextTexture = m_swapChain.getCurrentTextureView();
    if (!m_nextTexture) {
        if (!m_nextTexture) {
            std::cerr << "Could not acquire the swap chain texture " << std::endl;
            return;
        }
    }
    CommandEncoderDescriptor commandEncoderDesc;
    commandEncoderDesc.setDefault();
    m_encoder = m_device.createCommandEncoder(commandEncoderDesc);

    //Create the renderPassDescriptor
    RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachmentCount = 1;

    //Create the renderPassColor Attachment for the renderPassDescriptor
    RenderPassColorAttachment renderPassColorAttachment;
    renderPassColorAttachment.view = m_nextTexture;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
    renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
    renderPassColorAttachment.clearValue = WGPUColor{0.1, 0.1, 0.1, 0.0};
    renderPassDesc.colorAttachments = &renderPassColorAttachment;

    renderPassDesc.timestampWriteCount = 0;
    renderPassDesc.timestampWrites = nullptr;
    renderPassDesc.nextInChain = nullptr;

    //Write Buffers
    m_queue.writeBuffer(m_vertexBuffer, 0, m_vertexData[0].positions.data(),
                        m_vertexData[0].positions.size() * sizeof(float));
    m_queue.writeBuffer(m_normalBuffer, 0, m_vertexData[0].renderNormals.data(),
                        m_vertexData[0].renderNormals.size() * sizeof(float));
    m_queue.writeBuffer(m_indexBuffer, 0, m_vertexData[0].triangles.data(),
                        m_vertexData[0].triangles.size() * sizeof(uint16_t));
    auto t = static_cast<float>(glfwGetTime()); // glfwGetTime returns a double
    m_queue.writeBuffer(m_uTimeBuffer, 0, &t, sizeof(float));
    m_queue.writeBuffer(m_mvpBuffer, 0, &m_mvpUniforms, sizeof(MyUniforms));

    // Create the view of the depth texture manipulated by the rasterizer
    TextureViewDescriptor depthTextureViewDesc;
    depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
    depthTextureViewDesc.baseArrayLayer = 0;
    depthTextureViewDesc.arrayLayerCount = 1;
    depthTextureViewDesc.baseMipLevel = 0;
    depthTextureViewDesc.mipLevelCount = 1;
    depthTextureViewDesc.dimension = TextureViewDimension::_2D;
    depthTextureViewDesc.format = m_depthBufferFormat;
    m_depthTextureView = m_depthBuffer.createView(depthTextureViewDesc);

    //Now we define an object to connect our depth texture to the render pipeline
    RenderPassDepthStencilAttachment depthStencilAttachment;
    //Here we set up clear/store operations
    depthStencilAttachment.view = m_depthTextureView;
    depthStencilAttachment.depthClearValue = 1.0f;
    depthStencilAttachment.depthLoadOp = LoadOp::Clear;
    depthStencilAttachment.depthStoreOp = StoreOp::Store;
    depthStencilAttachment.depthReadOnly = false; //These affects to the Z-buffer globally (not only the texture)

    //We have to set the stencil state too
    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilLoadOp = LoadOp::Clear;
    depthStencilAttachment.stencilStoreOp = StoreOp::Store;
    depthStencilAttachment.stencilReadOnly = true;

    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

    m_renderPass = m_encoder.beginRenderPass(renderPassDesc);
    m_renderPass.setPipeline(m_renderPipeline);
    m_renderPass.setVertexBuffer(0, m_vertexBuffer, 0, m_vertexData[0].positions.size() * sizeof(float));
    m_renderPass.setVertexBuffer(1, m_normalBuffer, 0, m_vertexData[0].renderNormals.size() * sizeof(float));
    m_renderPass.setIndexBuffer(m_indexBuffer, IndexFormat::Uint16, 0,
                                m_vertexData[0].triangles.size() * sizeof(uint16_t));
    m_renderPass.setBindGroup(0, m_bindGroup, 0, nullptr);
    m_renderPass.drawIndexed(m_idxCount, 1, 0, 0, 0);
    m_renderPass.end();

    CommandBufferDescriptor cmdBuffDesc = Default;
    CommandBuffer renderCommand = m_encoder.finish(cmdBuffDesc);
    m_queue.submit(1, &renderCommand);
    m_encoder.release();

    //Destroy the texture view once used
    m_nextTexture.release();

    m_swapChain.present();
}

void Application::onFinish() {
    glfwTerminate();//Terminate te glfw process
    m_instance.release();
    m_shaderModule.release();
    m_renderPipeline.release();
    m_bindGroup.release();
    m_bindGroupLayout.release();
    m_swapChain.release();//Clean up the device swap chain
    m_queue.release(); //Clean up the device queue
    m_device.release(); //Clean up the adapters device
    m_adapter.release();  //Clean un the instance adapter
    m_surface.release();  //Clean up the WGPU surface
    m_instance.release(); //Clean up the WGPU instance
    m_uTimeBuffer.release();
    m_vertexBuffer.release();
    m_indexBuffer.release();
    m_normalBuffer.release();
    m_mvpBuffer.release();
    m_depthBuffer.destroy();
    m_depthBuffer.release();

    if (m_xBuffer != nullptr) m_xBuffer.release();
    if (m_piBuffer != nullptr) m_piBuffer.release();
    if (m_pfBuffer != nullptr) m_pfBuffer.release();
    if (m_vBuffer != nullptr) m_vBuffer.release();
    if (m_fBuffer != nullptr) m_fBuffer.release();
    if (m_stepDataBuffer != nullptr) m_stepDataBuffer.release();
    for (size_t i = 0; i < m_idxBuffer.size(); ++i) {
        if (m_idxBuffer[i] != nullptr) m_idxBuffer[i].release();
        if (m_dataBuffer[i] != nullptr) m_dataBuffer[i].release();
        if (m_stenCountBuffer[i] != nullptr) m_stenCountBuffer[i].release();
        if (m_stretchConstraint_BindPipe[i].bindGroup != nullptr) m_stretchConstraint_BindPipe[i].bindGroup.release();
        if (m_stretchConstraint_BindPipe[i].pipeline != nullptr) m_stretchConstraint_BindPipe[i].pipeline.release();
    }
    if (m_computeP_BindPipe.bindGroup != nullptr) m_computeP_BindPipe.bindGroup.release();
    if (m_computeP_BindPipe.pipeline != nullptr) m_computeP_BindPipe.pipeline.release();
    if (m_computeV_BindPipe.bindGroup != nullptr) m_computeV_BindPipe.bindGroup.release();
    if (m_computeV_BindPipe.pipeline != nullptr) m_computeV_BindPipe.pipeline.release();
    if (m_computeBindGroupLayout != nullptr) m_computeBindGroupLayout.release();
}

bool Application::initWindowAndDevice(int width, int height) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_window = glfwCreateWindow(width, height, "WGPU_PS", nullptr, nullptr);  //Create a window

    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    lastX = (float) x;
    lastY = (float) y;

    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow *window, int, int) {
        auto that = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
        if (that != nullptr) that->onResize();
    });
    glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double xPos, double yPos) {
        auto that = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
        if (that != nullptr) that->onMouseMove(xPos, yPos);
    });
    glfwSetKeyCallback(m_window, [](GLFWwindow *window, int key, int, int action, int) {
        auto that = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
        if (that != nullptr) that->onKeyPressed(key, action);
    });


    /////////////////////////////
    //////  WGPU INSTANCE  //////
    /////////////////////////////
    std::cout << "Getting the instance..." << std::endl;

    //Test the installation
    InstanceDescriptor instDesc;
    instDesc.setDefault();

    m_instance = createInstance(instDesc);

    if (!m_instance) {
        std::cerr << "Could not initialize WebGPU!" << std::endl;
        return false;
    }

    std::cout << "Instance successfully initialized!" << std::endl;

    /////////////////////////////
    //////  WGPU ADAPTER   //////
    /////////////////////////////
    std::cout << "Getting the adapter..." << std::endl;

    RequestAdapterOptions adapterOpts;
    adapterOpts.setDefault();
    adapterOpts.powerPreference = WGPUPowerPreference::WGPUPowerPreference_HighPerformance;
    m_surface = glfwGetWGPUSurface(m_instance, m_window);
    adapterOpts.compatibleSurface = m_surface;

    m_adapter = m_instance.requestAdapter(adapterOpts);
//    AdapterProperties adapter_prop;
//    m_adapter.getProperties(&adapter_prop);
//    printf("WGPUAdapterProperties {\n""\tvendorID: %" PRIu32 "\n""\tvendorName: %s\n""\tarchitecture: %s\n"
//           "\tdeviceID: %" PRIu32 "\n""\tname: %s\n""\tdriverDescription: %s\n""\tadapterType: %#.8x\n"
//           "\tbackendType: %#.8x\n""}\n",
//           adapter_prop.vendorID, adapter_prop.vendorName, adapter_prop.architecture, adapter_prop.deviceID,
//           adapter_prop.name, adapter_prop.driverDescription, adapter_prop.adapterType,
//           adapter_prop.backendType);

    std::cout << "Adapter successfully initialized!" << std::endl;

    std::cout << "Requesting the device..." << std::endl;

    DeviceDescriptor deviceDesc;
    deviceDesc.setDefault();

    //Here we get the capabilities of our device
    SupportedLimits supportedLimits;
    m_adapter.getLimits(&supportedLimits);

    //Now we set the required limits for our application
    RequiredLimits requiredLimits = Default;
    requiredLimits.limits.maxVertexAttributes = 3;
    requiredLimits.limits.maxVertexBuffers = 2;
    requiredLimits.limits.maxInterStageShaderComponents = 10;
    requiredLimits.limits.maxBufferSize = supportedLimits.limits.maxBufferSize;
    requiredLimits.limits.maxVertexBufferArrayStride = sizeof(Object);
    requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
    requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    requiredLimits.limits.maxUniformBufferBindingSize = sizeof(MyUniforms);
    requiredLimits.limits.maxBindingsPerBindGroup = 2;
    requiredLimits.limits.maxBindGroups = 2;
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 2;

    // For the depth buffer, we enable textures (up to the size of the window):
    requiredLimits.limits.maxTextureDimension1D = supportedLimits.limits.maxTextureDimension1D;
    requiredLimits.limits.maxTextureDimension2D = supportedLimits.limits.maxTextureDimension2D;
    requiredLimits.limits.maxTextureArrayLayers = 1;
    deviceDesc.requiredLimits = &requiredLimits;

    //Compute limits
    //Compute Buffers limits
    requiredLimits.limits.maxStorageBuffersPerShaderStage = 6;
    requiredLimits.limits.maxStorageBufferBindingSize = supportedLimits.limits.maxStorageBufferBindingSize;

    //Compute Texture limit
    requiredLimits.limits.maxSampledTexturesPerShaderStage = 10;
    requiredLimits.limits.maxStorageTexturesPerShaderStage = 2;

    // The maximum value for respectively w, h and d
    requiredLimits.limits.maxComputeWorkgroupSizeX = 128;
    requiredLimits.limits.maxComputeWorkgroupSizeY = 1;
    requiredLimits.limits.maxComputeWorkgroupSizeZ = 1;

    // The maximum value of the product w * h * d
    requiredLimits.limits.maxComputeInvocationsPerWorkgroup = 128;

    // And the maximum value of max(x, y, z)
    // (It is 2 because workgroupCount = 64 / 32 = 2)
    requiredLimits.limits.maxComputeWorkgroupsPerDimension = 128;

    m_device = m_adapter.requestDevice(deviceDesc);

    //Set an error message for the device
    auto onDeviceError = [](ErrorType type, char const *message) {
        std::cout << "Uncaptured device error: type " << type;
        if (message) std::cout << " " << message;
        std::cout << std::endl;
    };
    m_errorCallbackHandle = m_device.setUncapturedErrorCallback(onDeviceError);

    m_device.getLimits(&supportedLimits);
    m_deviceLimits = supportedLimits.limits;
    std::cout << "Device successfully initialized!" << std::endl;

    return true;
}

void Application::initSwapChain() {
//    std::cout << "Asking the device for a swap chain..." << std::endl;

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    //We need to tell the swap chain some of the characteristics of the textures we will be using
    SwapChainDescriptor swapChainDesc;
    swapChainDesc.height = static_cast<uint32_t>(height);
    swapChainDesc.width = static_cast<uint32_t>(width);
    m_SwapChainFormat = m_surface.getPreferredFormat(m_adapter);
    swapChainDesc.format = m_SwapChainFormat;
    swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
    swapChainDesc.presentMode = WGPUPresentMode_Fifo;

    m_swapChain = m_device.createSwapChain(m_surface, swapChainDesc);

//    std::cout << "Got the Swap Chain!" << std::endl;
}

void Application::createPipeline() {
    m_shaderModule = ResourceManager::loadShaderModule(RESOURCE_DIR "/shader.wgsl", m_device);
    m_pipelineData.setVertexDescription(m_shaderModule);
    m_pipelineData.setPrimitiveDescriptor();
    m_pipelineData.setFragmentDescriptor(m_SwapChainFormat, m_shaderModule);
    m_pipelineData.setMisc();
    m_renderPipeline = m_device.createRenderPipeline(m_pipelineData.pipeDesc);
}

void Application::initDepthBuffer() {
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    m_depthBufferFormat = TextureFormat::Depth24Plus;
    m_pipelineData.setDepthStencilDescriptor(m_depthBufferFormat);

    //Now we need to allocate the texture to store de Z-buffer
    TextureDescriptor depthTextureDesc;
    depthTextureDesc.dimension = TextureDimension::_2D;
    depthTextureDesc.format = m_depthBufferFormat;
    depthTextureDesc.mipLevelCount = 1;
    depthTextureDesc.sampleCount = 1;
    depthTextureDesc.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    depthTextureDesc.usage = TextureUsage::RenderAttachment;
    depthTextureDesc.viewFormatCount = 1;
    depthTextureDesc.viewFormats = (WGPUTextureFormat *) &m_depthBufferFormat;
    m_depthBuffer = m_device.createTexture(depthTextureDesc);

}

void Application::initBindings() {
    // Create binding layout (don't forget to = Default)
    m_bindingLayoutEntries.resize(2);
    m_bindingLayoutEntries[0] = Default;
    m_bindingLayoutEntries[0].binding = 0;
    m_bindingLayoutEntries[0].visibility = ShaderStage::Vertex;
    m_bindingLayoutEntries[0].buffer.type = BufferBindingType::Uniform;
    m_bindingLayoutEntries[0].buffer.minBindingSize = sizeof(float);

    m_bindingLayoutEntries[1] = Default;
    m_bindingLayoutEntries[1].binding = 1;
    m_bindingLayoutEntries[1].visibility = ShaderStage::Vertex | ShaderStage::Fragment;
    m_bindingLayoutEntries[1].buffer.type = BufferBindingType::Uniform;
    m_bindingLayoutEntries[1].buffer.minBindingSize = sizeof(MyUniforms);

    // Create a bind group layout
    BindGroupLayoutDescriptor bindGroupLayoutDesc = Default;
    bindGroupLayoutDesc.entryCount = 2;
    bindGroupLayoutDesc.entries = m_bindingLayoutEntries.data();
    m_bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutDesc);

    // Create the pipeline layout
    PipelineLayoutDescriptor layoutDesc = Default;
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout *) &m_bindGroupLayout;
    PipelineLayout layout = m_device.createPipelineLayout(layoutDesc);
    m_pipelineData.pipeDesc.layout = layout;

    // Create a binding
    std::vector<BindGroupEntry> binding(2);
    binding[0] = Default;
    binding[0].binding = 0;
    binding[0].buffer = m_uTimeBuffer;
    binding[0].offset = 0;
    binding[0].size = sizeof(float);

    binding[1] = Default;
    binding[1].binding = 1;
    binding[1].buffer = m_mvpBuffer;
    binding[1].offset = 0;
    binding[1].size = sizeof(MyUniforms);

    // A bind group contains one or multiple bindings
    BindGroupDescriptor bindGroupDesc = Default;
    bindGroupDesc.layout = m_bindGroupLayout;
    // There must be as many bindings as declared in the layout!
    bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
    bindGroupDesc.entries = binding.data();
    m_bindGroup = m_device.createBindGroup(bindGroupDesc);
}

void Application::initBuffers() {
    BufferDescriptor bufferDesc;
    bufferDesc.size = m_vertexData[0].positions.size() * sizeof(float);
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex;
    bufferDesc.mappedAtCreation = false;
    m_vertexBuffer = m_device.createBuffer(bufferDesc);

    bufferDesc.size = m_vertexData[0].renderNormals.size() * sizeof(double);
    m_normalBuffer = m_device.createBuffer(bufferDesc);

    bufferDesc.size = m_vertexData[0].triangles.size() * sizeof(uint16_t);
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Index;
    m_indexBuffer = m_device.createBuffer(bufferDesc);

    bufferDesc.size = sizeof(float);
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
    m_uTimeBuffer = m_device.createBuffer(bufferDesc);

    bufferDesc.size = sizeof(MyUniforms);
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
    m_mvpBuffer = m_device.createBuffer(bufferDesc);
}

void Application::onResize() {
    initSwapChain();
    initDepthBuffer();
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
//    std::cout<< "Width: " << width << "    Height: " << height << std::endl;
    float ratio = (float) width / (float) height;
    m_mvpUniforms.projectionMatrix = glm::perspective(glm::radians(60.0f), ratio, 0.01f, 100.0f);
}

void Application::onMouseMove(double x, double y) {

    float xOffset = ((float) x - lastX) * sensitivity;
    float yOffset = (lastY - (float) y) * sensitivity; //Reversed because y coordinates go from button to top
    lastX = (float) x;
    lastY = (float) y;

    yaw += xOffset;
    pitch += yOffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = (float) cos(glm::radians(yaw) * cos(glm::radians(pitch)));
    front.y = (float) sin(glm::radians(pitch));
    front.z = (float) sin(glm::radians(yaw) * cos(glm::radians(pitch)));
    m_camState.front = glm::normalize(m_camState.pos + front);

    m_mvpUniforms.viewMatrix = glm::lookAt(m_camState.pos, m_camState.front, m_camState.up);

//    std::cout << "X: " << x << "   Y: " << y << std::endl;

}

void Application::onKeyPressed(int key, int action) {

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(m_window, true);
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
//        physicManager.unPause();
//        PhysicManagerPBD::unPause();
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
//        physicManagerPbd.step();
    }
}

Application::Application(std::vector<Object> &vData) : m_computeP_BindPipe({nullptr, nullptr}),
                                                       m_computeV_BindPipe({nullptr, nullptr}), m_vertexData(vData) {
    m_camState.pos = glm::vec3(0.f);
    m_camState.front = glm::vec3(0.f, 0.f, -1.f);
    m_camState.up = glm::vec3(0.f, 1.f, 0.f);
    m_mvpUniforms.viewMatrix = glm::lookAt(m_camState.pos, m_camState.front, m_camState.up);
    m_numDof = 0;
}

ComputePipeline Application::createComputePipeline(ComputeOperation shader) {
    std::string entry_point;
    switch (shader) {
        case PROJECT_STRETCH:
            m_computeShaderModule = ResourceManager::loadShaderModule(RESOURCE_DIR "/computeShader.wgsl", m_device);
            entry_point = "projectStretchConstraint";
            break;
        case COMPUTE_P:
            m_computeShaderModule = ResourceManager::loadShaderModule(RESOURCE_DIR "/computeP.wgsl", m_device);
            entry_point = "computePredictedPos";
            break;
        case COMPUTE_V:
            m_computeShaderModule = ResourceManager::loadShaderModule(RESOURCE_DIR "/computeV.wgsl", m_device);
            entry_point = "update_V_X";
            break;
    }

    ComputePipelineDescriptor computePipelineDesc = Default;
    computePipelineDesc.compute.entryPoint = entry_point.c_str();
    computePipelineDesc.compute.module = m_computeShaderModule;
    computePipelineDesc.layout = m_computePipelineLayout;
    return m_device.createComputePipeline(computePipelineDesc);
}

BindGroup Application::createConstraintBindings(size_t idx_size, size_t data_size, int color) {

    //We set the amount of layout entries
    m_computeBindingLayoutEntries.resize(5);

    //Pi Buffer
    m_computeBindingLayoutEntries[0].binding = 0;
    m_computeBindingLayoutEntries[0].visibility = ShaderStage::Compute;
    m_computeBindingLayoutEntries[0].buffer.minBindingSize = m_numDof * sizeof(float);
    m_computeBindingLayoutEntries[0].buffer.type = BufferBindingType::ReadOnlyStorage;
    m_computeBindingLayoutEntries[0].buffer.hasDynamicOffset = false;

    //Pf Buffer
    m_computeBindingLayoutEntries[1].binding = 1;
    m_computeBindingLayoutEntries[1].visibility = ShaderStage::Compute;
    m_computeBindingLayoutEntries[1].buffer.minBindingSize = m_numDof * sizeof(float);
    m_computeBindingLayoutEntries[1].buffer.type = BufferBindingType::Storage;
    m_computeBindingLayoutEntries[1].buffer.hasDynamicOffset = false;

    //Index Buffer
    m_computeBindingLayoutEntries[2].binding = 2;
    m_computeBindingLayoutEntries[2].visibility = ShaderStage::Compute;
    m_computeBindingLayoutEntries[2].buffer.minBindingSize = idx_size * sizeof(uint32_t);
    m_computeBindingLayoutEntries[2].buffer.type = BufferBindingType::ReadOnlyStorage;
    m_computeBindingLayoutEntries[2].buffer.hasDynamicOffset = false;

    //Data Buffer
    m_computeBindingLayoutEntries[3].binding = 3;
    m_computeBindingLayoutEntries[3].visibility = ShaderStage::Compute;
    m_computeBindingLayoutEntries[3].buffer.minBindingSize = data_size * sizeof(float);
    m_computeBindingLayoutEntries[3].buffer.type = BufferBindingType::ReadOnlyStorage;
    m_computeBindingLayoutEntries[3].buffer.hasDynamicOffset = false;

    //Data Size Buffer
    m_computeBindingLayoutEntries[4].binding = 4;
    m_computeBindingLayoutEntries[4].visibility = ShaderStage::Compute;
    m_computeBindingLayoutEntries[4].buffer.type = BufferBindingType::ReadOnlyStorage;
    m_computeBindingLayoutEntries[4].buffer.minBindingSize = sizeof(uint32_t);

    //We create the layout group
    BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entryCount = (uint32_t) m_computeBindingLayoutEntries.size();
    bindGroupLayoutDesc.entries = m_computeBindingLayoutEntries.data();
    m_computeBindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutDesc);

    //Create the pipeline layout
    PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout *) &m_computeBindGroupLayout;
    m_computePipelineLayout = m_device.createPipelineLayout(pipelineLayoutDesc);

    //Now we create the bind group
    std::vector<BindGroupEntry> entries(5, Default);

    //Pi Buffer
    entries[0].binding = 0;
    entries[0].buffer = m_piBuffer;
    entries[0].size = sizeof(float) * m_numDof;
    entries[0].offset = 0;

    //Pf Buffer
    entries[1].binding = 1;
    entries[1].buffer = m_pfBuffer;
    entries[1].size = sizeof(float) * m_numDof;
    entries[1].offset = 0;

    //Idx Buffer
    entries[2].binding = 2;
    entries[2].buffer = m_idxBuffer[color];
    entries[2].size = sizeof(uint32_t) * idx_size;
    entries[2].offset = 0;

    //Data Buffer
    entries[3].binding = 3;
    entries[3].buffer = m_dataBuffer[color];
    entries[3].size = sizeof(float) * data_size;
    entries[3].offset = 0;

    //Size Buffer
    entries[4].binding = 4;
    entries[4].buffer = m_stenCountBuffer[color];
    entries[4].size = sizeof(uint32_t);
    entries[4].offset = 0;

    BindGroupDescriptor bindGroupDesc;
    bindGroupDesc.layout = m_computeBindGroupLayout;
    bindGroupDesc.entryCount = entries.size();
    bindGroupDesc.entries = (WGPUBindGroupEntry *) entries.data();
    return m_device.createBindGroup(bindGroupDesc);

}

uint32_t Application::respectAlignment(uint32_t size) {
    uint32_t align_remainder = size % m_deviceLimits.minStorageBufferOffsetAlignment;
    if (align_remainder != 0) {
        align_remainder = m_deviceLimits.minStorageBufferOffsetAlignment - align_remainder;
    }
    return size + align_remainder;
}


void Application::computeStretch(int color_count) {

    CommandEncoderDescriptor encoderDesc = Default;
    CommandEncoder encoder = nullptr;

    ComputePassDescriptor computePassDesc;
    computePassDesc.timestampWriteCount = 0;
    computePassDesc.timestampWrites = nullptr;

    for (int i = 0; i < color_count; ++i) {

        encoder = m_device.createCommandEncoder(encoderDesc);
        m_computePass = encoder.beginComputePass(computePassDesc);
        m_computePass.setPipeline(m_stretchConstraint_BindPipe[i].pipeline);
        m_computePass.setBindGroup(0, m_stretchConstraint_BindPipe[i].bindGroup, 0, nullptr);
        m_computePass.dispatchWorkgroups(128, 1, 1);
        m_computePass.end();
        if (i != color_count - 1) {
            encoder.copyBufferToBuffer(m_pfBuffer, 0, m_piBuffer, 0, m_numDof * sizeof(float));
        }
        CommandBuffer commands = encoder.finish(CommandBufferDescriptor{});
        m_queue.submit(commands);
        encoder.release();
    }
}

void Application::readP(VectorXR &p) {

    CommandEncoderDescriptor encoderDesc = Default;
    CommandEncoder encoder = nullptr;

    encoder = m_device.createCommandEncoder(encoderDesc);
    //Now we want to copy the texture to our mapped buffer
    encoder.copyBufferToBuffer(m_pfBuffer, 0, m_mapBuffer, 0, m_numDof * sizeof(float));

    CommandBuffer commands = encoder.finish(CommandBufferDescriptor{});
    m_queue.submit(commands);
    encoder.release();


    bool done = false;
    auto handle = m_mapBuffer.mapAsync(MapMode::Read, 0, m_numDof * sizeof(float),
                                       [&](BufferMapAsyncStatus status) {
        if (status == BufferMapAsyncStatus::Success) {
            const auto *output = (const float *) m_mapBuffer.getConstMappedRange(0,m_numDof *sizeof(float));
            Eigen::Map<const VectorXR> newP(output, m_numDof);
            p = newP;
            m_mapBuffer.unmap();
        }
        done = true;
    });


    while (!done) {
#ifdef WEBGPU_BACKEND_WGPU
        m_queue.submit(0, nullptr);
#else
        m_instance.processEvents();
#endif
    }


    // Clean up
#if !defined(WEBGPU_BACKEND_WGPU)
    wgpuCommandBufferRelease(commands);
    wgpuCommandEncoderRelease(encoder);
    wgpuQueueRelease(m_queue);
    wgpuComputePassEncoderRelease(computePass);
#endif

}

void Application::computeSimulation(ComputeOperation compute_operation) {
    CommandEncoderDescriptor encoderDesc = Default;
    CommandEncoder encoder = nullptr;

    ComputePassDescriptor computePassDesc;
    computePassDesc.timestampWriteCount = 0;
    computePassDesc.timestampWrites = nullptr;


    encoder = m_device.createCommandEncoder(encoderDesc);
    m_computePass = encoder.beginComputePass(computePassDesc);
    switch (compute_operation) {

        case COMPUTE_P:
            m_computePass.setPipeline(m_computeP_BindPipe.pipeline);
            m_computePass.setBindGroup(0, m_computeP_BindPipe.bindGroup, 0, nullptr);
            break;
        case COMPUTE_V:
            m_computePass.setPipeline(m_computeV_BindPipe.pipeline);
            m_computePass.setBindGroup(0, m_computeV_BindPipe.bindGroup, 0, nullptr);
            break;
        case PROJECT_STRETCH:
            break;
    }

    m_computePass.dispatchWorkgroups(128, 1, 1);
    m_computePass.end();
    CommandBuffer commands = encoder.finish(CommandBufferDescriptor{});
    m_queue.submit(commands);
    encoder.release();
}

BindGroup Application::createSimulationBindings(ComputeOperation compute_operation) {
    size_t binding_count;
    switch (compute_operation) {
        case COMPUTE_P:
            binding_count = 6;
            break;
        case COMPUTE_V:
            binding_count = 4;
            break;
        case PROJECT_STRETCH:
            break;
    }

    //We set the amount of layout entries
    m_computeBindingLayoutEntries.resize(binding_count);

    //X Buffer
    m_computeBindingLayoutEntries[0].binding = 0;
    m_computeBindingLayoutEntries[0].visibility = ShaderStage::Compute;
    m_computeBindingLayoutEntries[0].buffer.minBindingSize = m_numDof * sizeof(float);
    if (compute_operation == ComputeOperation::COMPUTE_P)
        m_computeBindingLayoutEntries[0].buffer.type = BufferBindingType::ReadOnlyStorage;
    else
        m_computeBindingLayoutEntries[0].buffer.type = BufferBindingType::Storage;
    m_computeBindingLayoutEntries[0].buffer.hasDynamicOffset = false;

    //Pf Buffer
    m_computeBindingLayoutEntries[1].binding = 1;
    m_computeBindingLayoutEntries[1].visibility = ShaderStage::Compute;
    m_computeBindingLayoutEntries[1].buffer.minBindingSize = m_numDof * sizeof(float);
    m_computeBindingLayoutEntries[1].buffer.type = BufferBindingType::Storage;
    m_computeBindingLayoutEntries[1].buffer.hasDynamicOffset = false;

    //V Buffer
    m_computeBindingLayoutEntries[2].binding = 2;
    m_computeBindingLayoutEntries[2].visibility = ShaderStage::Compute;
    m_computeBindingLayoutEntries[2].buffer.minBindingSize = m_numDof * sizeof(float);
    m_computeBindingLayoutEntries[2].buffer.type = BufferBindingType::Storage;
    m_computeBindingLayoutEntries[2].buffer.hasDynamicOffset = false;

    //Step Data Buffer
    m_computeBindingLayoutEntries[3].binding = 3;
    m_computeBindingLayoutEntries[3].visibility = ShaderStage::Compute;
    m_computeBindingLayoutEntries[3].buffer.minBindingSize = sizeof(StepData);
    m_computeBindingLayoutEntries[3].buffer.type = BufferBindingType::ReadOnlyStorage;
    m_computeBindingLayoutEntries[3].buffer.hasDynamicOffset = false;

    if (compute_operation == ComputeOperation::COMPUTE_P) {
        //F Buffer
        m_computeBindingLayoutEntries[4].binding = 4;
        m_computeBindingLayoutEntries[4].visibility = ShaderStage::Compute;
        m_computeBindingLayoutEntries[4].buffer.minBindingSize = m_numDof * sizeof(float);
        m_computeBindingLayoutEntries[4].buffer.type = BufferBindingType::ReadOnlyStorage;
        m_computeBindingLayoutEntries[4].buffer.hasDynamicOffset = false;

        //Pi Buffer
        m_computeBindingLayoutEntries[5].binding = 5;
        m_computeBindingLayoutEntries[5].visibility = ShaderStage::Compute;
        m_computeBindingLayoutEntries[5].buffer.minBindingSize = m_numDof * sizeof(float);
        m_computeBindingLayoutEntries[5].buffer.type = BufferBindingType::Storage;
        m_computeBindingLayoutEntries[5].buffer.hasDynamicOffset = false;
    }

    //We create the layout group
    BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entryCount = (uint32_t) m_computeBindingLayoutEntries.size();
    bindGroupLayoutDesc.entries = m_computeBindingLayoutEntries.data();
    m_computeBindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutDesc);

    //Create the pipeline layout
    PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout *) &m_computeBindGroupLayout;
    m_computePipelineLayout = m_device.createPipelineLayout(pipelineLayoutDesc);

    //Now we create the bind group
    std::vector<BindGroupEntry> entries(binding_count, Default);

    //X Buffer
    entries[0].binding = 0;
    entries[0].buffer = m_xBuffer;
    entries[0].size = sizeof(float) * m_numDof;
    entries[0].offset = 0;

    //Pf Buffer
    entries[1].binding = 1;
    entries[1].buffer = m_pfBuffer;
    entries[1].size = sizeof(float) * m_numDof;
    entries[1].offset = 0;

    //V Buffer
    entries[2].binding = 2;
    entries[2].buffer = m_vBuffer;
    entries[2].size = sizeof(uint32_t) * m_numDof;
    entries[2].offset = 0;

    //Step Data Buffer
    entries[3].binding = 3;
    entries[3].buffer = m_stepDataBuffer;
    entries[3].size = sizeof(StepData);
    entries[3].offset = 0;

    if (compute_operation == ComputeOperation::COMPUTE_P) {
        //F Buffer
        entries[4].binding = 4;
        entries[4].buffer = m_fBuffer;
        entries[4].size = sizeof(float) * m_numDof;
        entries[4].offset = 0;

        //Pi Buffer
        entries[5].binding = 5;
        entries[5].buffer = m_piBuffer;
        entries[5].size = sizeof(float) * m_numDof;
        entries[5].offset = 0;
    }

    BindGroupDescriptor bindGroupDesc;
    bindGroupDesc.layout = m_computeBindGroupLayout;
    bindGroupDesc.entryCount = entries.size();
    bindGroupDesc.entries = (WGPUBindGroupEntry *) entries.data();
    return m_device.createBindGroup(bindGroupDesc);
}

void Application::initSimulationStruct(VectorXR &x, VectorXR &v, VectorXR &f, float ts) {
    m_numDof = int(x.size());

    ///////////////////////////////////////////
    //////   CREATE AND WRITE BUFFERS    //////
    ///////////////////////////////////////////
    //Map Buffer for reading the texture
    BufferDescriptor buffDesc;

    //Map Buffer
    buffDesc.mappedAtCreation = false;
    buffDesc.size = m_numDof * sizeof(float);
    buffDesc.usage = BufferUsage::CopyDst | BufferUsage::MapRead;
    buffDesc.mappedAtCreation = false;
    m_mapBuffer = m_device.createBuffer(buffDesc);

    //X Buffer
    buffDesc.size = m_numDof * sizeof(float);
    buffDesc.usage = BufferUsage::Storage | BufferUsage::CopySrc | BufferUsage::CopyDst;
    m_xBuffer = m_device.createBuffer(buffDesc);

    //Pi Buffer
    buffDesc.size = m_numDof * sizeof(float);
    buffDesc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    m_piBuffer = m_device.createBuffer(buffDesc);

    //Pf Buffer
    buffDesc.size = m_numDof * sizeof(float);
    buffDesc.usage = BufferUsage::Storage | BufferUsage::CopySrc | BufferUsage::CopyDst;
    m_pfBuffer = m_device.createBuffer(buffDesc);

    //V Buffer
    buffDesc.size = m_numDof * sizeof(float);
    buffDesc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    m_vBuffer = m_device.createBuffer(buffDesc);

    //F Buffer
    buffDesc.size = m_numDof * sizeof(float);
    buffDesc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    m_fBuffer = m_device.createBuffer(buffDesc);

    //Time Step Buffer
    buffDesc.size = sizeof(StepData);
    buffDesc.usage = BufferUsage::Storage | BufferUsage::CopyDst;
    m_stepDataBuffer = m_device.createBuffer(buffDesc);

    VectorXR zero(m_numDof);
    zero.setZero();
    m_queue.writeBuffer(m_piBuffer, 0, zero.data(), m_numDof * sizeof(float));
    m_queue.writeBuffer(m_pfBuffer, 0, zero.data(), m_numDof * sizeof(float));
    m_queue.writeBuffer(m_xBuffer, 0, x.data(), m_numDof * sizeof(float));
    m_queue.writeBuffer(m_vBuffer, 0, v.data(), m_numDof * sizeof(float));
    m_queue.writeBuffer(m_fBuffer, 0, f.data(), m_numDof * sizeof(float));
    StepData stepData{};
    stepData.num_dof = m_numDof;
    stepData.time_step = ts;
    m_queue.writeBuffer(m_stepDataBuffer, 0, &stepData, sizeof(StepData));

    ////////////////////////////////////////////
    /////   CREATE PIPELINE & BINDINGS    //////
    ////////////////////////////////////////////
    //COMPUTE_P BINDING AND PIPELINE
    m_computeP_BindPipe.bindGroup = createSimulationBindings(ComputeOperation::COMPUTE_P);
    m_computeP_BindPipe.pipeline = createComputePipeline(ComputeOperation::COMPUTE_P);

    //COMPUTE_V BINDING AND PIPELINE
    m_computeV_BindPipe.bindGroup = createSimulationBindings(ComputeOperation::COMPUTE_V);
    m_computeV_BindPipe.pipeline = createComputePipeline(ComputeOperation::COMPUTE_V);
}

void
Application::initConstraintsStruct(ConstraintType constraint, std::vector<Vector32i> &id, std::vector<VectorXR> &data) {

    static_cast<void>(constraint);
    ///////////////////////////////////////////
    //////   CREATE AND WRITE BUFFERS    //////
    ///////////////////////////////////////////

    //We now create a buffer for each color
    BufferDescriptor buffDesc;

    size_t color_count = id.size();
    m_idxBuffer.reserve(color_count);
    m_dataBuffer.reserve(color_count);
    m_stenCountBuffer.reserve(color_count);

    size_t idx_size, data_size;
    for (size_t i = 0; i < color_count; ++i) {
        idx_size = id[i].size();
        data_size = data[i].size();

        //Data Buffer
        buffDesc.size = data_size * sizeof(float);
        buffDesc.usage = BufferUsage::CopyDst | BufferUsage::Storage;
        m_dataBuffer[i] = m_device.createBuffer(buffDesc);

        //Index Buffer
        buffDesc.size = idx_size * sizeof(uint32_t);
        buffDesc.usage = BufferUsage::CopyDst | BufferUsage::Storage;
        m_idxBuffer[i] = m_device.createBuffer(buffDesc);

        //Compute size buffer
        buffDesc.size = sizeof(uint32_t);
        buffDesc.usage = BufferUsage::CopyDst | BufferUsage::Storage;
        m_stenCountBuffer[i] = m_device.createBuffer(buffDesc);
    }


    m_idxSizes.reserve(color_count);
    m_dataSizes.reserve(color_count);
    for (int i = 0; i < int(color_count); ++i) {
        m_queue.writeBuffer(m_idxBuffer[i], 0, id[i].data(), id[i].size() * sizeof(uint32_t));
        m_idxSizes[i] = int(id[i].size());

        m_queue.writeBuffer(m_dataBuffer[i], 0, data[i].data(), data[i].size() * sizeof(float));
        m_dataSizes[i] = int(data[i].size());

        uint32_t constraintCount = id[i].size() / 2;
        m_queue.writeBuffer(m_stenCountBuffer[i], 0, &constraintCount, sizeof(uint32_t));
    }

    ////////////////////////////////////////////
    /////   CREATE PIPELINE & BINDINGS    //////
    ////////////////////////////////////////////
    m_stretchConstraint_BindPipe.reserve(color_count);

    for (int i = 0; i < int(color_count); ++i) {
        idx_size = id[i].size();
        data_size = data[i].size();
        m_stretchConstraint_BindPipe[i].bindGroup = createConstraintBindings(idx_size, data_size, i);
        m_stretchConstraint_BindPipe[i].pipeline = createComputePipeline(ComputeOperation::PROJECT_STRETCH);
    }

}
