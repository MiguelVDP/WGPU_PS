//
// Created by Miguel on 21/10/2023.
//
#include "application.h"

using namespace wgpu;

bool Application::onInit(int width, int height) {
    if (!glfwInit()) {  // Initialize GLFW & check for any GLFW error
        std::cout << "Could not initialize GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(width, height, "WGPU_PS", nullptr, nullptr);  //Create a window

    if (!initInstanceAdapter()) return false;

    initDeviceLimits(width, height);

    m_queue = m_device.getQueue();

    initSwapChain(width, height);

    createDepthTexture();

    initBuffers();

    initBindings();

    createPipeline();

    m_idxCount = static_cast<int>(m_vertexData.size());

    if (!m_window) {  //Check for errors
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return false;
    }

    return true;
}

void Application::onFrame() {

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
    renderPassColorAttachment.clearValue = WGPUColor{ 0.1, 0.1, 0.1, 0.0 };
    renderPassDesc.colorAttachments = &renderPassColorAttachment;

    renderPassDesc.timestampWriteCount = 0;
    renderPassDesc.timestampWrites = nullptr;
    renderPassDesc.nextInChain = nullptr;

    //Write Buffers
    m_queue.writeBuffer(m_vertexBuffer, 0, m_vertexData.data(), m_vertexData.size() * sizeof(VertexAttributes));
    auto t = static_cast<float>(glfwGetTime()); // glfwGetTime returns a double
    m_queue.writeBuffer(m_uTimeBuffer, 0, &t, sizeof(float ));
    m_queue.writeBuffer(m_mvpBuffer, 0, &m_mvpUniforms, sizeof(MyUniforms));

    // Create the view of the depth texture manipulated by the rasterizer
    TextureViewDescriptor depthTextureViewDesc;
    depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
    depthTextureViewDesc.baseArrayLayer = 0;
    depthTextureViewDesc.arrayLayerCount = 1;
    depthTextureViewDesc.baseMipLevel = 0;
    depthTextureViewDesc.mipLevelCount = 1;
    depthTextureViewDesc.dimension = TextureViewDimension::_2D;
    depthTextureViewDesc.format = m_depthTextureFormat;
    m_depthTextureView = m_depthTexture.createView(depthTextureViewDesc);

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
    m_renderPass.setVertexBuffer(0, m_vertexBuffer, 0, m_vertexData.size() * sizeof(VertexAttributes));
    m_renderPass.setBindGroup(0, m_bindGroup, 0, nullptr);
    m_renderPass.draw(m_idxCount, 1, 0,0);
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
    m_mvpBuffer.release();
    m_depthTexture.destroy();
    m_depthTexture.release();
}

bool Application::initInstanceAdapter() {
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

    return true;
}

void Application::initDeviceLimits(int width, int height) {
    std::cout << "Requesting the device..." << std::endl;

    DeviceDescriptor deviceDesc;
    deviceDesc.setDefault();

    //Here we get the capabilities of our device
    SupportedLimits supportedLimits;
    m_adapter.getLimits(&supportedLimits);

    //Now we set the required limits for our application
    RequiredLimits requiredLimits = Default;
    requiredLimits.limits.maxVertexAttributes = 3;
    requiredLimits.limits.maxVertexBuffers = 1;
    requiredLimits.limits.maxInterStageShaderComponents = 6;
    requiredLimits.limits.maxBufferSize = 1000 * sizeof(VertexAttributes);
    requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexAttributes);
    requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
    requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    requiredLimits.limits.maxUniformBufferBindingSize = sizeof(MyUniforms);
    requiredLimits.limits.maxBindingsPerBindGroup = 2;
    requiredLimits.limits.maxBindGroups = 2;
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 2;
    // For the depth buffer, we enable textures (up to the size of the window):
    requiredLimits.limits.maxTextureDimension1D = height;
    requiredLimits.limits.maxTextureDimension2D = width;
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
}

void Application::initSwapChain(int width, int height) {
    std::cout << "Asking the device for a swap chain..." << std::endl;

    //We need to tell the swap chain some of the characteristics of the textures we will be using
    SwapChainDescriptor swapChainDesc;
    swapChainDesc.height = height;
    swapChainDesc.width = width;
    m_SwapChainFormat = m_surface.getPreferredFormat(m_adapter);
    swapChainDesc.format = m_SwapChainFormat;
    swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
    swapChainDesc.presentMode = WGPUPresentMode_Fifo;

    m_swapChain = m_device.createSwapChain(m_surface, swapChainDesc);

    std::cout << "Got the Swap Chain!" << std::endl;
}

void Application::createPipeline() {
    m_shaderModule = ResourceManager::loadShaderModule(RESOURCE_DIR "/shader.wgsl", m_device);
    m_pipelineData.setVertexDescription(m_shaderModule);
    m_pipelineData.setPrimitiveDescriptor();
    m_pipelineData.setFragmentDescriptor(m_SwapChainFormat, m_shaderModule);
    m_pipelineData.setMisc();
    m_renderPipeline = m_device.createRenderPipeline(m_pipelineData.pipeDesc);
}

void Application::createDepthTexture() {
    m_depthTextureFormat = TextureFormat::Depth24Plus;
    m_pipelineData.setDepthStencilDescriptor(m_depthTextureFormat);

    //Now we need to allocate the texture to store de Z-buffer
    TextureDescriptor depthTextureDesc;
    depthTextureDesc.dimension = TextureDimension::_2D;
    depthTextureDesc.format = m_depthTextureFormat;
    depthTextureDesc.mipLevelCount = 1;
    depthTextureDesc.sampleCount = 1;
    depthTextureDesc.size = {640, 480, 1};
    depthTextureDesc.usage = TextureUsage::RenderAttachment;
    depthTextureDesc.viewFormatCount = 1;
    depthTextureDesc.viewFormats = (WGPUTextureFormat*)&m_depthTextureFormat;
    m_depthTexture = m_device.createTexture(depthTextureDesc);

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
    layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&m_bindGroupLayout;
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
    bufferDesc.size = m_vertexData.size() * sizeof(VertexAttributes);
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex;
    bufferDesc.mappedAtCreation = false;
    m_vertexBuffer = m_device.createBuffer(bufferDesc);

    bufferDesc.size = sizeof(float);
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
    m_uTimeBuffer = m_device.createBuffer(bufferDesc);

    bufferDesc.size = sizeof(MyUniforms);
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
    m_mvpBuffer = m_device.createBuffer(bufferDesc);
}


