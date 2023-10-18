#include <iostream>
#include <glfw/glfw3.h>
#include <glfw3webgpu.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <webgpu/webgpu.hpp>
#include <pipelineData.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace wgpu;
namespace fs = std::filesystem;

int width = 640;
int height = 480;

struct MyUniforms {
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;
    glm::mat4 model2Matrix;
};

bool loadGeometry(const fs::path& path, std::vector<float>& pointData, std::vector<uint16_t>& indexData, int dimensions) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    pointData.clear();
    indexData.clear();

    enum class Section {
        None,
        Points,
        Indices,
    };
    Section currentSection = Section::None;

    float value;
    uint16_t index;
    std::string line;
    while (!file.eof()) {
        getline(file, line);

        // overcome the `CRLF` problem
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line == "[points]") {
            currentSection = Section::Points;
        }
        else if (line == "[indices]") {
            currentSection = Section::Indices;
        }
        else if (line[0] == '#' || line.empty()) {
            // Do nothing, this is a comment
        }
        else if (currentSection == Section::Points) {
            std::istringstream iss(line);
            // Get x, y, r, g, b
            for (int i = 0; i < dimensions + 3; ++i) {
                iss >> value;
                pointData.push_back(value);
            }
        }
        else if (currentSection == Section::Indices) {
            std::istringstream iss(line);
            // Get corners #0 #1 and #2
            for (int i = 0; i < 3; ++i) {
                iss >> index;
                indexData.push_back(index);
            }
        }
    }
    return true;
}

ShaderModule loadShaderModule(const fs::path& path, Device device) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return nullptr;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string shaderSource(size, ' ');
    file.seekg(0);
    file.read(shaderSource.data(), size);

    ShaderModuleWGSLDescriptor shaderCodeDesc{};
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
    shaderCodeDesc.code = shaderSource.c_str();
    ShaderModuleDescriptor shaderDesc{};
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    return device.createShaderModule(shaderDesc);
}

void transformVertex(MyUniforms &uniforms, float t){
    float angle = t;
    glm::mat4  m = glm::mat4(1.0f);
    m = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -1, 0));
    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(t,0,0));
    m = glm::rotate(m, angle, glm::vec3(0, 0, 1));
    uniforms.modelMatrix = m;
}

void transformVertex2(MyUniforms &uniforms, float t){
    float angle = t;
    glm::mat4  m = glm::mat4(1.0f);
    m = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0, 0));
    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(t,0,0));
    m = glm::rotate(m, angle, glm::vec3(0, 0, 1));
    uniforms.model2Matrix = m;
}

std::vector<float> vertexData;
std::vector<uint16_t> indexData;

int main() {
    if (!glfwInit()) {  // Initialize GLFW & check for any GLFW error
        std::cout << "Could not initialize GLFW!" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(width, height, "WGPU_PS", nullptr, nullptr);  //Create a window

    /////////////////////////////
    //////  WGPU INSTANCE  //////
    /////////////////////////////
    std::cout << "Getting the instance..." << std::endl;

    //Test the installation
    InstanceDescriptor instDesc;
    instDesc.setDefault();

    Instance instance = createInstance(instDesc);

    if (!instance) {
        std::cerr << "Could not initialize WebGPU!" << std::endl;
        return 1;
    }

    std::cout << "Instance successfully initialized!" << std::endl;

    /////////////////////////////
    //////  WGPU ADAPTER   //////
    /////////////////////////////
    std::cout << "Getting the adapter..." << std::endl;

    RequestAdapterOptions adapterOpts;
    adapterOpts.setDefault();
    Surface surface = glfwGetWGPUSurface(instance, window);
    adapterOpts.compatibleSurface = surface;

    Adapter adapter = instance.requestAdapter(adapterOpts);

    std::cout << "Adapter successfully initialized!" << std::endl;

//    size_t featureCount = adapter.enumerateFeatures(nullptr);
//    auto *features = reinterpret_cast<FeatureName *>(malloc(sizeof(FeatureName) * featureCount));
//    adapter.enumerateFeatures(features);
//    std::cout << "The adapter detected the following features: " << std::endl;
//    for (int i = 0; i < featureCount; i++) {
//        std::cout << " - " << features[i] << std::endl;
//    }

    /////////////////////////////
    //////   WGPU DEVICE   //////
    /////////////////////////////
    std::cout << "Requesting the device..." << std::endl;

    DeviceDescriptor deviceDesc;
    deviceDesc.setDefault();

    //Here we get the capabilities of our device
    SupportedLimits supportedLimits;
    adapter.getLimits(&supportedLimits);

    //Now we set the required limits for our application
    RequiredLimits requiredLimits = Default;
    requiredLimits.limits.maxVertexAttributes = 2;
    requiredLimits.limits.maxVertexBuffers = 1;
    requiredLimits.limits.maxInterStageShaderComponents = 3;
    requiredLimits.limits.maxBufferSize = 16 * 5 * sizeof(float);
    requiredLimits.limits.maxVertexBufferArrayStride = 6 * sizeof(float);
    requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
    requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    requiredLimits.limits.maxUniformBufferBindingSize = sizeof(MyUniforms);
    requiredLimits.limits.maxBindingsPerBindGroup = 2;
    requiredLimits.limits.maxBindGroups = 2;
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 2;
    // For the depth buffer, we enable textures (up to the size of the window):
    requiredLimits.limits.maxTextureDimension1D = 480;
    requiredLimits.limits.maxTextureDimension2D = 640;
    requiredLimits.limits.maxTextureArrayLayers = 1;
    deviceDesc.requiredLimits = &requiredLimits;

    Device device = adapter.requestDevice(deviceDesc);

    //Set an error message for the device
    auto onDeviceError = [](ErrorType type, char const *message) {
        std::cout << "Uncaptured device error: type " << type;
        if (message) std::cout << " " << message;
        std::cout << std::endl;
    };
    auto h = device.setUncapturedErrorCallback(onDeviceError);

    std::cout << "Device successfully initialized!" << std::endl;

    /////////////////////////////
    ////  WGPU COMMAND QUEUE ////
    /////////////////////////////
    Queue queue = device.getQueue();

    /////////////////////////////
    ////// WGPU SWAP CHAIN //////
    /////////////////////////////
    std::cout << "Asking the device for a swap chain..." << std::endl;

    //We need to tell the swap chain some of the characteristics of the textures we will be using
    SwapChainDescriptor swapChainDesc;
    swapChainDesc.height = height;
    swapChainDesc.width = width;
    WGPUTextureFormat swapChainFormat = surface.getPreferredFormat(adapter);
    swapChainDesc.format = swapChainFormat;
    swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
    swapChainDesc.presentMode = WGPUPresentMode_Fifo;

    SwapChain swapChain = device.createSwapChain(surface, swapChainDesc);

    std::cout << "Got the Swap Chain!" << std::endl;

    if (!window) {  //Check for errors
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return 1;
    }

    /////////////////////////////
    ////// SHADER CREATION //////
    /////////////////////////////

    std::cout << "Creating shader module..." << std::endl;
    ShaderModule shaderModule = loadShaderModule(RESOURCE_DIR "/shader.wgsl", device);
    std::cout << "Shader module created: " << shaderModule << std::endl;

    //Read vertex and index from file
    bool success = loadGeometry(RESOURCE_DIR "/pyramid.txt", vertexData, indexData, 3);
    if (!success) {
        std::cerr << "Could not load geometry!" << std::endl;
        return 1;
    }

    //Manage the uniforms her
    MyUniforms uniforms{};
    uniforms.projectionMatrix = glm::mat4(0.f);

    float aspectRatio = (float)width / (float)height;
    float near = 0.01f;
    float far = 20.f;
    float fov = glm::radians(60.0f);

    uniforms.projectionMatrix = glm::perspective(fov, aspectRatio, near, far);

    uniforms.modelMatrix = glm::translate(glm::mat4x4(1.0f), glm::vec3(0, 0, 0));
    uniforms.modelMatrix = glm::mat4(1.0f);
    uniforms.viewMatrix = glm::mat4x4(1.0f);
    uniforms.viewMatrix[3].z = -4.f;


    //Main window loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ///Get the target texture view
        TextureView nextTexture = swapChain.getCurrentTextureView();
        if(!nextTexture) {
            std::cerr << "Could not acquire the swap chain texture " << std::endl;
            break;
        }

        ///Draw in the texture
        CommandEncoderDescriptor commandEncoderDesc;
        commandEncoderDesc.setDefault();
        CommandEncoder encoder = device.createCommandEncoder(commandEncoderDesc);

        //Create the renderPassDescriptor
        RenderPassDescriptor renderPassDesc;
        renderPassDesc.colorAttachmentCount = 1;

        //Create the renderPassColor Attachment for the renderPassDescriptor
        RenderPassColorAttachment renderPassColorAttachment;
        renderPassColorAttachment.view = nextTexture;
        renderPassColorAttachment.resolveTarget = nullptr;
        renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
        renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
        renderPassColorAttachment.clearValue = WGPUColor{ 0.1, 0.1, 0.1, 0.0 };
        renderPassDesc.colorAttachments = &renderPassColorAttachment;

        renderPassDesc.depthStencilAttachment = nullptr;
        renderPassDesc.timestampWriteCount = 0;
        renderPassDesc.timestampWrites = nullptr;
        renderPassDesc.nextInChain = nullptr;

        /////////////////////////////
        ////// BUFFER CREATION //////
        /////////////////////////////

        //Create a vertex buffer
        BufferDescriptor bufferDesc;
        bufferDesc.size = vertexData.size() * sizeof(float);
        bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex;
        bufferDesc.mappedAtCreation = false;
        Buffer vertexBuffer = device.createBuffer(bufferDesc);
        // Upload geometry data to the buffer
        queue.writeBuffer(vertexBuffer, 0, vertexData.data(), bufferDesc.size);

        //Create the index buffer
        bufferDesc.size = indexData.size() * sizeof(float);
        bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Index;
        Buffer indexBuffer = device.createBuffer(bufferDesc);
        queue.writeBuffer(indexBuffer, 0, indexData.data(), bufferDesc.size);

        //Create the uniform buffer
        bufferDesc.size = sizeof(float);
        bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
        Buffer uniformBuffer = device.createBuffer(bufferDesc);
        // Update uniform buffer
        auto t = static_cast<float>(glfwGetTime()); // glfwGetTime returns a double
        queue.writeBuffer(uniformBuffer, 0, &t, sizeof(float));

        bufferDesc.size = sizeof(MyUniforms);
        bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
        Buffer mvpBuffer = device.createBuffer(bufferDesc);
        transformVertex(uniforms, t);
        transformVertex2(uniforms, t);
        queue.writeBuffer(mvpBuffer, 0, &uniforms, sizeof(MyUniforms));

        /////////////////////////////
        ///// PIPELINE CREATION /////
        /////////////////////////////
        PipelineData pipelineData;
        pipelineData.setVertexDescription(shaderModule, 2);
        pipelineData.setPrimitiveDescriptor();
        pipelineData.setFragmentDescriptor(swapChainFormat, shaderModule);

        /////////////   DEPTH STENCIL STATE /////////////
        TextureFormat depthTextureFormat = TextureFormat::Depth24Plus;
        pipelineData.setDepthStencilDescriptor(depthTextureFormat);

        //Now we need to allocate the texture to store de Z-buffer
        TextureDescriptor depthTextureDesc;
        depthTextureDesc.dimension = TextureDimension::_2D;
        depthTextureDesc.format = depthTextureFormat;
        depthTextureDesc.mipLevelCount = 1;
        depthTextureDesc.sampleCount = 1;
        depthTextureDesc.size = {640, 480, 1};
        depthTextureDesc.usage = TextureUsage::RenderAttachment;
        depthTextureDesc.viewFormatCount = 1;
        depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
        Texture depthTexture = device.createTexture(depthTextureDesc);

        //We need to create a view for the render pipeline to use
        // Create the view of the depth texture manipulated by the rasterizer
        TextureViewDescriptor depthTextureViewDesc;
        depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
        depthTextureViewDesc.baseArrayLayer = 0;
        depthTextureViewDesc.arrayLayerCount = 1;
        depthTextureViewDesc.baseMipLevel = 0;
        depthTextureViewDesc.mipLevelCount = 1;
        depthTextureViewDesc.dimension = TextureViewDimension::_2D;
        depthTextureViewDesc.format = depthTextureFormat;
        TextureView depthTextureView = depthTexture.createView(depthTextureViewDesc);

        //Now we define an object to connect our depth texture to the render pipeline
        RenderPassDepthStencilAttachment depthStencilAttachment;
        //Here we set up clear/store operations
        depthStencilAttachment.view = depthTextureView;
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
        ////////////////////////////////////////////////////////////
        // Create binding layout (don't forget to = Default)
        std::vector<BindGroupLayoutEntry> bindingLayout(2);
        bindingLayout[0] = Default;
        bindingLayout[0].binding = 0;
        bindingLayout[0].visibility = ShaderStage::Vertex;
        bindingLayout[0].buffer.type = BufferBindingType::Uniform;
        bindingLayout[0].buffer.minBindingSize = sizeof(float);

        bindingLayout[1] = Default;
        bindingLayout[1].binding = 1;
        bindingLayout[1].visibility = ShaderStage::Vertex | ShaderStage::Fragment;
        bindingLayout[1].buffer.type = BufferBindingType::Uniform;
        bindingLayout[1].buffer.minBindingSize = sizeof(MyUniforms);

        // Create a bind group layout
        BindGroupLayoutDescriptor bindGroupLayoutDesc = Default;
        bindGroupLayoutDesc.entryCount = 2;
        bindGroupLayoutDesc.entries = bindingLayout.data();
        BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

        // Create the pipeline layout
        PipelineLayoutDescriptor layoutDesc = Default;
        layoutDesc.bindGroupLayoutCount = 1;
        layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
        PipelineLayout layout = device.createPipelineLayout(layoutDesc);
        pipelineData.pipeDesc.layout = layout;

        // Create a binding
        std::vector<BindGroupEntry> binding(2);
        binding[0] = Default;
        binding[0].binding = 0;
        binding[0].buffer = uniformBuffer;
        binding[0].offset = 0;
        binding[0].size = sizeof(float);

        binding[1] = Default;
        binding[1].binding = 1;
        binding[1].buffer = mvpBuffer;
        binding[1].offset = 0;
        binding[1].size = sizeof(MyUniforms);

        // A bind group contains one or multiple bindings
        BindGroupDescriptor bindGroupDesc = Default;
        bindGroupDesc.layout = bindGroupLayout;
        // There must be as many bindings as declared in the layout!
        bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
        bindGroupDesc.entries = binding.data();
        BindGroup bindGroup = device.createBindGroup(bindGroupDesc);

        pipelineData.setMisc();

        RenderPipeline pipeline = device.createRenderPipeline(pipelineData.pipeDesc);
        RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
        renderPass.setPipeline(pipeline);
        renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexBuffer.getSize());
        renderPass.setIndexBuffer(indexBuffer, IndexFormat::Uint16, 0, indexData.size() * sizeof(uint16_t));
        int indexCount = static_cast<int>(indexData.size());
        renderPass.setBindGroup(0, bindGroup, 0, nullptr);
        renderPass.drawIndexed(indexCount, 2, 0, 0,0);
        renderPass.end();

        CommandBufferDescriptor cmdBuffDesc = Default;
        CommandBuffer renderCommand = encoder.finish(cmdBuffDesc);
        queue.submit(1, &renderCommand);
        encoder.release();

        //Destroy the texture view once used
        nextTexture.release();
        // Destroy the depth texture and its view
        depthTextureView.release();
        depthTexture.destroy();
        depthTexture.release();
        uniformBuffer.release();
        indexBuffer.release();
        vertexBuffer.release();
        mvpBuffer.release();

        ///Swap the textures
        swapChain.present();
    }

    glfwTerminate();//Terminate te glfw process
    swapChain.release();//Clean up the device swap chain
    queue.release(); //Clean up the device queue
    device.release(); //Clean up the adapters device
    adapter.release();  //Clean un the instance adapter
    surface.release();  //Clean up the WGPU surface
    instance.release(); //Clean up the WGPU instance

    return 0;
}
