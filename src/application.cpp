#include "application.h"

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
    m_encoder = m_device.createCommandEncoder();

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
    m_renderPass.setIndexBuffer(m_indexBuffer, IndexFormat::Uint16, 0, m_vertexData[0].triangles.size() * sizeof(uint16_t));
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
    m_surface = glfwGetWGPUSurface(m_instance, m_window);
    adapterOpts.compatibleSurface = m_surface;

    m_adapter = m_instance.requestAdapter(adapterOpts);

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
    requiredLimits.limits.maxInterStageShaderComponents = 6;
    requiredLimits.limits.maxBufferSize = 10000 * sizeof(double);
    requiredLimits.limits.maxVertexBufferArrayStride = sizeof(Object);
    requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
    requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    requiredLimits.limits.maxUniformBufferBindingSize = sizeof(MyUniforms);
    requiredLimits.limits.maxBindingsPerBindGroup = 2;
    requiredLimits.limits.maxBindGroups = 2;
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 2;
    // For the depth buffer, we enable textures (up to the size of the window):
    requiredLimits.limits.maxTextureDimension1D = 2160;
    requiredLimits.limits.maxTextureDimension2D = 3840;
    requiredLimits.limits.maxTextureArrayLayers = 1;
    deviceDesc.requiredLimits = &requiredLimits;

    m_device = m_adapter.requestDevice(deviceDesc);

    //Set an error message for the device
    auto onDeviceError = [](ErrorType type, char const *message) {
        std::cout << "Uncaptured device error: type " << type;
        if (message) std::cout << " " << message;
        std::cout << std::endl;
    };
    m_errorCallbackHandle = m_device.setUncapturedErrorCallback(onDeviceError);

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
    front.x = cos(glm::radians(yaw) * cos(glm::radians(pitch)));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw) * cos(glm::radians(pitch)));
    m_camState.front = glm::normalize(m_camState.pos + front);

    m_mvpUniforms.viewMatrix = glm::lookAt(m_camState.pos, m_camState.front, m_camState.up);

//    std::cout << "X: " << x << "   Y: " << y << std::endl;

}

void Application::onKeyPressed(int key, int action) {

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(m_window, true);
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        physicManager.unPause();
    }

//    float cameraSpeed = camSpeed * deltaTime;
//    if(key == GLFW_KEY_W) m_camState.pos += m_camState.front * cameraSpeed;
//    if(key == GLFW_KEY_S) m_camState.pos -= m_camState.front * cameraSpeed;
//    if(key == GLFW_KEY_D) m_camState.pos += glm::normalize(glm::cross(m_camState.front, m_camState.up)) * cameraSpeed;
//    if(key == GLFW_KEY_A) m_camState.pos -= glm::normalize(glm::cross(m_camState.front, m_camState.up)) * cameraSpeed;
//
//    m_mvpUniforms.viewMatrix = glm::lookAt(m_camState.pos, m_camState.front, m_camState.up);
}

Application::Application(std::vector<Object> &vData, PhysicManager &manager) : m_vertexData(vData),
                                                                               physicManager(manager) {
    m_camState.pos = glm::vec3(0.f);
    m_camState.front = glm::vec3(0.f, 0.f, -1.f);
    m_camState.up = glm::vec3(0.f, 1.f, 0.f);
    m_mvpUniforms.viewMatrix = glm::lookAt(m_camState.pos, m_camState.front, m_camState.up);
}


