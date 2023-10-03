#include <iostream>
#include <glfw/glfw3.h>
#include <glfw3webgpu.h>

#define WEBGPU_CPP_IMPLEMENTATION

#include "webgpu/webgpu.hpp"

using namespace wgpu;

int width = 640;
int height = 480;


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


// Vertex buffer
// There are 2 floats per vertex, one for x and one for y.
// But in the end this is just a bunch of floats to the eyes of the GPU,
// the *layout* will tell how to interpret this.
std::vector<float> vertexData = {
        // x,   y,     r,   g,   b
        -0.5, -0.5,   1.0, 0.0, 0.0,
        +0.5, -0.5,   0.0, 1.0, 0.0,
        +0.5, +0.5,   0.0, 0.0, 1.0,
        -0.5, +0.5,   1.0, 1.0, 0.0
};

std::vector<uint16_t> indexData = {
        0, 1, 2, // Triangle #0
        0, 2, 3  // Triangle #1
};


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
    requiredLimits.limits.maxBufferSize = 6 * 5 * sizeof(float);
    requiredLimits.limits.maxVertexBufferArrayStride = 5 * sizeof(float);
    requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
    requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
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

    const char* shaderSource = R"(
struct VertexInput {
    @location(0) position: vec2f,
    @location(1) color: vec3f,
};

/**
 * A structure with fields labeled with builtins and locations can also be used
 * as *output* of the vertex shader, which is also the input of the fragment
 * shader.
 */
struct VertexOutput {
    @builtin(position) position: vec4f,
    // The location here does not refer to a vertex attribute, it just means
    // that this field must be handled by the rasterizer.
    // (It can also refer to another field of another struct that would be used
    // as input to the fragment shader.)
    @location(0) color: vec3f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4f(in.position, 0.0, 1.0);
    out.color = in.color; // forward to the fragment shader
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.color, 1.0);
})";
    ShaderModuleDescriptor shaderDesc;
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
    ShaderModuleWGSLDescriptor shaderCodeDesc;
    // Set the chained struct's header
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
    // Connect the chain
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    shaderCodeDesc.code = shaderSource;

    ShaderModule shaderModule = device.createShaderModule(shaderDesc);


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
        renderPassColorAttachment.clearValue = WGPUColor{ 0.9, 0.1, 0.2, 1.0 };
        renderPassDesc.colorAttachments = &renderPassColorAttachment;

        renderPassDesc.depthStencilAttachment = nullptr;
        renderPassDesc.timestampWriteCount = 0;
        renderPassDesc.timestampWrites = nullptr;
        renderPassDesc.nextInChain = nullptr;

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

        //Stencil and depth
        pipelineDesc.depthStencil = nullptr;

        // Samples per pixel
        pipelineDesc.multisample.count = 1;
        // Default value for the mask, meaning "all bits on"
        pipelineDesc.multisample.mask = ~0u;
        // Default value as well (irrelevant for count = 1 anyways)
        pipelineDesc.multisample.alphaToCoverageEnabled = false;

        pipelineDesc.layout = nullptr;

        RenderPipeline pipeline = device.createRenderPipeline(pipelineDesc);
        RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
        renderPass.setPipeline(pipeline);
        renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexBuffer.getSize());
        renderPass.setIndexBuffer(indexBuffer, IndexFormat::Uint16, 0, indexData.size() * sizeof(uint16_t));
        int indexCount = static_cast<int>(indexData.size());
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
