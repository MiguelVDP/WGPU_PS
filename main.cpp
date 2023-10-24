#include <iostream>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <structs.h>
#include <application.h>

using namespace wgpu;
namespace fs = std::filesystem;

void transformVertex(Application &app, float t){
    float angle = t;
    glm::mat4  m = glm::mat4(1.0f);
    m = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -1, -0));
    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(t,0,0));
    m = glm::rotate(m, angle, glm::vec3(0, 0, 1));
    app.m_mvpUniforms.modelMatrix = m;
}

void transformVertex2(Application &app, float t){
    float angle = t;
    glm::mat4  m = glm::mat4(1.0f);
    m = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -1, 0));
    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(t,0,0));
    m = glm::rotate(m, angle, glm::vec3(0, 0, 1));
    m = glm::translate(m, glm::vec3(0.f, -1.5, 0));
    m = glm::rotate(m, angle, glm::vec3(0, 0, 1));

    app.m_mvpUniforms.model2Matrix = m;
}


int main() {
    Application app;

    ResourceManager::loadGeometryFromObj(RESOURCE_DIR "/pyramid.obj", app.m_vertexData);
    app.onInit(640, 480);

    MyUniforms uniforms{};
    uniforms.projectionMatrix = glm::mat4(0.f);

    float ratio = 640.f / 480.f;
    float near = 0.01f;
    float far = 100.f;
    float fov = glm::radians(60.0f);

    uniforms.projectionMatrix = glm::perspective(fov, ratio, near, far);

    uniforms.modelMatrix = glm::mat4(1.0f);
    uniforms.model2Matrix = glm::mat4(1.0f);
    uniforms.viewMatrix = glm::mat4x4(1.0f);
    uniforms.viewMatrix[3].z = -4.f;
    app.set_MVPUniforms(uniforms);

    while(app.isRunning()){
        auto t = static_cast<float>(glfwGetTime()); // glfwGetTime returns a double
        transformVertex(app, t);
        transformVertex2(app, t);
        app.onFrame();
    }

    app.onFinish();

    return 0;
}
