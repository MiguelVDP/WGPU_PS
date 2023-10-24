#include <iostream>
#include <glfw/glfw3.h>
#include <glfw3webgpu.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <webgpu/webgpu.hpp>
#include <pipelineData.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_obj_loader.h>
#include <structs.h>
#include <application.h>

using namespace wgpu;
namespace fs = std::filesystem;

int width = 640;
int height = 480;


void transformVertex(MyUniforms &uniforms, float t){
    float angle = t;
    glm::mat4  m = glm::mat4(1.0f);
    m = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -1, -0));
    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(t,0,0));
    m = glm::rotate(m, angle, glm::vec3(0, 0, 1));
    uniforms.modelMatrix = m;
}

void transformVertex2(MyUniforms &uniforms, float t){
    float angle = t;
    glm::mat4  m = glm::mat4(1.0f);
    m = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -1, 0));
    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(t,0,0));
    m = glm::rotate(m, angle, glm::vec3(0, 0, 1));
    m = glm::translate(m, glm::vec3(0.f, -1.5, 0));
    m = glm::rotate(m, angle, glm::vec3(0, 0, 1));

    uniforms.model2Matrix = m;
}


int main() {
    Application app;

    ResourceManager::loadGeometryFromObj(RESOURCE_DIR "/pyramid.obj", app.m_vertexData);
    app.onInit(width, height);

    MyUniforms uniforms{};
    uniforms.projectionMatrix = glm::mat4(0.f);

    float aspectRatio = (float)width / (float)height;
    float near = 0.01f;
    float far = 20.f;
    float fov = glm::radians(60.0f);

    uniforms.projectionMatrix = glm::perspective(fov, aspectRatio, near, far);

    uniforms.modelMatrix = glm::mat4(1.0f);
    uniforms.model2Matrix = glm::mat4(1.0f);
    uniforms.viewMatrix = glm::mat4x4(1.0f);
    uniforms.viewMatrix[3].z = -4.f;


    while(app.isRunning()){
        auto t = static_cast<float>(glfwGetTime()); // glfwGetTime returns a double
        transformVertex(uniforms, t);
        transformVertex2(uniforms, t);
        app.set_MVPUniforms(uniforms);
        app.onFrame();
    }

    app.onFinish();

    return 0;
}
