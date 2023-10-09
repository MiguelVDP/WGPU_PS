#include <iostream>
#include <glfw/glfw3.h>
#include <glfw3webgpu.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <pipelineData.h>

#define WEBGPU_CPP_IMPLEMENTATION

#include "webgpu/webgpu.hpp"

using namespace wgpu;
namespace fs = std::filesystem;

int width = 640;
int height = 480;

bool loadGeometry(const fs::path& path, std::vector<float>& pointData, std::vector<uint16_t>& indexData) {
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
            for (int i = 0; i < 5; ++i) {
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


void setVertexDescription(RenderPipelineDescriptor &pipeDesc, ShaderModule shaderModule,
                          VertexBufferLayout &vertexBufferLayout, std::vector<VertexAttribute>& vertexAttribs){

    //The shader source code
    pipeDesc.vertex.module = shaderModule;
    //The function entry point in the shader
    pipeDesc.vertex.entryPoint = "vs_main";
    //Vertex constants
    pipeDesc.vertex.constantCount = 0;
    pipeDesc.vertex.constants = nullptr;

    // == Per attribute ==
    //Position
    vertexAttribs[0].shaderLocation = 0;  // Corresponds to @location(...)
    vertexAttribs[0].format = VertexFormat::Float32x2;
    vertexAttribs[0].offset = 0;
    //Color
    vertexAttribs[1].shaderLocation = 1;
    vertexAttribs[1].format = VertexFormat::Float32x3;
    vertexAttribs[1].offset = 2 * sizeof(float);

    // == Common to attributes from the same buffer ==
    vertexBufferLayout.stepMode = VertexStepMode::Vertex;
    vertexBufferLayout.attributeCount = static_cast<uint32_t>(vertexAttribs.size());
    vertexBufferLayout.attributes = vertexAttribs.data();
    vertexBufferLayout.arrayStride = 5 * sizeof(float);
    //Buffers from the information will be sent
    pipeDesc.vertex.bufferCount = 1;
    pipeDesc.vertex.buffers = &vertexBufferLayout;
}


void setPrimitiveDescriptor(RenderPipelineDescriptor &pipeDesc){
    // Each sequence of 3 vertices is considered as a triangle
    pipeDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;

    //Order in which the vertices should be read, if undefined, they will be read sequentially
    pipeDesc.primitive.stripIndexFormat = IndexFormat::Undefined;

    //Triangle orientation, (CCW = counter clock wise)
    pipeDesc.primitive.frontFace = WGPUFrontFace_CCW;

    //Set the cull mode (this process avoid painting back faces
    pipeDesc.primitive.cullMode = CullMode::None;
}


void setFragmentDescriptor(RenderPipelineDescriptor &pipeDesc, FragmentState &fragmentState, BlendState &blendState,
                           ColorTargetState &colorTarget, WGPUTextureFormat swapChainFormat, ShaderModule shaderModule){
    /////////////////////
    /////  FRAGMENT  ////
    /////////////////////
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;

    /////////////////////
    /////   BLEND   /////
    /////////////////////

    //As the fragment shader is in charge of colouring the fragments, we should configure the blending (color mixing)
    //Here we use an alpha blending equation
    blendState.color.srcFactor = BlendFactor::SrcAlpha;
    blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = BlendOperation::Add;
    //We set the alpha channel untouched
    blendState.alpha.srcFactor = BlendFactor::Zero;
    blendState.alpha.dstFactor = BlendFactor::One;
    blendState.alpha.operation = BlendOperation::Add;

    /////////////////////
    //// COLOR TARGET ///
    /////////////////////
    //The color target will define the format and behaviour of the color targets this pipeline writes to
    colorTarget.format = swapChainFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = ColorWriteMask::All;

    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    pipeDesc.fragment = &fragmentState;
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
    requiredLimits.limits.maxBufferSize = 15 * 5 * sizeof(float);
    requiredLimits.limits.maxVertexBufferArrayStride = 5 * sizeof(float);
    requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
    requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
    requiredLimits.limits.maxBindGroups = 1;
    requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4;
    deviceDesc.requiredLimits = &requiredLimits;

    Device device = adapter.requestDevice(deviceDesc);

    //Set an error message for the device
    auto onDeviceError = [](ErrorType type, char const *message) {
        std::cout << "Uncaptured device error: type " << type;
        if (message) std::cout << " " << message;
        std::cout << std::endl;
    };
    device.setUncapturedErrorCallback(onDeviceError);

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
    bool success = loadGeometry(RESOURCE_DIR "/webgpu.txt", vertexData, indexData);
    if (!success) {
        std::cerr << "Could not load geometry!" << std::endl;
        return 1;
    }

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
        renderPassColorAttachment.clearValue = WGPUColor{ 0.1, 0.1, 0.1, 1.0 };
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

        /////////////////////////////
        ///// PIPELINE CREATION /////
        /////////////////////////////
        PipelineData pipelineData;
        pipelineData.setVertexDescription(shaderModule, 2);
        pipelineData.setPrimitiveDescriptor();
        pipelineData.setFragmentDescriptor(swapChainFormat, shaderModule);
        //Set a pipeline for the RenderPass
        RenderPipelineDescriptor pipelineDesc;
        VertexBufferLayout vertexBufferLayout;
        std::vector<VertexAttribute> vertexAttribs(2);
        setVertexDescription(pipelineDesc, shaderModule, vertexBufferLayout, vertexAttribs);
        setPrimitiveDescriptor(pipelineDesc);

        FragmentState fragmentState;
        BlendState blendState;
        ColorTargetState colorTarget;
        setFragmentDescriptor(pipelineDesc,fragmentState,blendState,colorTarget,swapChainFormat, shaderModule);

        // Create binding layout (don't forget to = Default)
        BindGroupLayoutEntry bindingLayout = Default;
        bindingLayout.binding = 0;
        bindingLayout.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
        bindingLayout.buffer.type = BufferBindingType::Uniform;
        bindingLayout.buffer.minBindingSize = sizeof(float);

        // Create a bind group layout
        BindGroupLayoutDescriptor bindGroupLayoutDesc = Default;
        bindGroupLayoutDesc.entryCount = 1;
        bindGroupLayoutDesc.entries = &bindingLayout;
        BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

        // Create the pipeline layout
        PipelineLayoutDescriptor layoutDesc = Default;
        layoutDesc.bindGroupLayoutCount = 1;
        layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
        PipelineLayout layout = device.createPipelineLayout(layoutDesc);
        pipelineDesc.layout = layout;
        pipelineData.pipeDesc.layout = layout;

        // Create a binding
        BindGroupEntry binding = Default;
        binding.binding = 0;
        binding.buffer = uniformBuffer;
        binding.offset = 0;
        binding.size = sizeof(float);

        // A bind group contains one or multiple bindings
        BindGroupDescriptor bindGroupDesc = Default;
        bindGroupDesc.layout = bindGroupLayout;
        // There must be as many bindings as declared in the layout!
        bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
        bindGroupDesc.entries = &binding;
        BindGroup bindGroup = device.createBindGroup(bindGroupDesc);


        //Stencil and depth
        pipelineDesc.depthStencil = nullptr;

        // Samples per pixel
        pipelineDesc.multisample.count = 1;
        // Default value for the mask, meaning "all bits on"
        pipelineDesc.multisample.mask = ~0u;
        // Default value as well (irrelevant for count = 1 anyway)
        pipelineDesc.multisample.alphaToCoverageEnabled = false;

        pipelineData.setMisc();

        RenderPipeline pipeline = device.createRenderPipeline(pipelineData.pipeDesc);
        RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
        renderPass.setPipeline(pipeline);
        renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexBuffer.getSize());
        renderPass.setIndexBuffer(indexBuffer, IndexFormat::Uint16, 0, indexData.size() * sizeof(uint16_t));
        int indexCount = static_cast<int>(indexData.size());
        renderPass.setBindGroup(0, bindGroup, 0, nullptr);
        renderPass.drawIndexed(indexCount, 1, 0, 0,0);
        renderPass.end();

        CommandBufferDescriptor cmdBuffDesc = Default;
        CommandBuffer renderCommand = encoder.finish(cmdBuffDesc);
        queue.submit(1, &renderCommand);
        encoder.release();

        //Destroy the texture view once used
        nextTexture.release();

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
