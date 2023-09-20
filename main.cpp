#include <iostream>
#include <glfw/glfw3.h>
#include <glfw3webgpu.h>

#define WEBGPU_CPP_IMPLEMENTATION

#include "webgpu/webgpu.hpp"

using namespace wgpu;

int width = 640;
int height = 480;

int main() {

    if (!glfwInit()) {  // Initialize GLFW & check for any GLFW error
        std::cout << "Could not initialize GLFW!" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(width, height, "WGPU_PS", NULL, NULL);  //Create a window

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
    WGPUTextureFormat textFormat = surface.getPreferredFormat(adapter);
    swapChainDesc.format = textFormat;
    swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
    swapChainDesc.presentMode = WGPUPresentMode_Fifo;

    SwapChain swapChain = device.createSwapChain(surface, swapChainDesc);

    std::cout << "Got the Swap Chain!" << std::endl;

    if (!window) {  //Check for errors
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return 1;
    }

    //Main window loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        //Get the target texture view
        TextureView nextTexture = swapChain.getCurrentTextureView();
        if(!nextTexture) {
            std::cerr << "Could not acquire the swap chain texture " << std::endl;
            break;
        }

        //Draw in the texture
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

        RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
        renderPass.end();

        CommandBufferDescriptor cmdBuffDesc = Default;
        CommandBuffer renderCommand = encoder.finish(cmdBuffDesc);
        queue.submit(1, &renderCommand);
        encoder.release();

        //Destroy the texture view once used
        nextTexture.release();
        //Swap the textures
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
