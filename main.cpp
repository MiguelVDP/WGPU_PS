#include <iostream>
#include <glfw/glfw3.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu/webgpu.hpp"

using namespace std;

int width = 640;
int height = 480;

int main() {

    //Test the installation


    if (!glfwInit()) {  // Initialize GLFW & check for any GLFW error
        std::cout << "Could not initialize GLFW!" << std::endl;
        return 1;
    }

    GLFWwindow *window = glfwCreateWindow(width, height, "WGPU_PS", NULL, NULL);  //Create a window

    if (!window) {  //Check for errors
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return 1;
    }

    //Main window loop
    while (!glfwWindowShouldClose(window)) {
        // Check whether the user clicked on the close button (and any other
        // mouse/key event, which we don't use so far)
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
