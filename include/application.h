#pragma once
#include <webgpu/webgpu.hpp>
#include <glfw/glfw3.h>
#include <glfw3webgpu/glfw3webgpu.h>
#include <structs.h>
#include <resourceManager.h>

class Application {
public:
    // A function called only once at the beginning. Returns false is init failed.
    bool onInit(int width, int height);

    // A function called at each frame, guaranteed never to be called before `onInit`.
    void onFrame();

    // A function called only once at the very end.
    void onFinish();

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
};
