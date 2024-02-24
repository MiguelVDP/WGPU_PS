#include <iostream>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <structs.h>
#include <application.h>
#include <Eigen/Dense>
#include <physicmanager.h>
#include <massSpring.h>
#include <PBD/physicmanagerPBD.h>
#include <PBD/massSpringPBD.h>
#include <chrono>

using namespace wgpu;
namespace fs = std::filesystem;

void transformVertex(Application &app, float t) {
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0, -5));
//    m = glm::rotate(m, glm::radians(90.0f), glm::vec3(1, 0, 0));
//    m = glm::rotate(m, t, glm::vec3(0, 0, 1));
    static_cast<void>(t);
    app.m_mvpUniforms.modelMatrix = m;
}

void transformVertex2(Application &app, float t) {
    float angle = t;
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -1, 0));
    m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(t, 0, 0));
    m = glm::rotate(m, angle, glm::vec3(0, 0, 1));
    m = glm::translate(m, glm::vec3(0.f, -1.5, 0));
    m = glm::rotate(m, angle, glm::vec3(0, 0, 1));

    app.m_mvpUniforms.model2Matrix = m;
}


int main() {

    std::vector<Object> objectData;
    PhysicManager physicManagerd;
    Application app(objectData);
    PhysicManagerPBD physicManager(app);


    ResourceManager::loadGeometryFromObj(RESOURCE_DIR "/plano.obj", objectData);

    for(auto &obj : objectData){
        obj.localToWorld();
    }

    physicManager.simObjs.emplace_back(
            std::unique_ptr<SimulablePBD>(new MassSpringPBD(0.5f, 5.f, 2.5f, physicManager, objectData[0])));
    physicManager.initialize();

    app.onInit(false);

    MyUniforms uniforms{};
    uniforms.projectionMatrix = glm::mat4(0.f);

    float ratio = 640.f / 480.f;
    float near = 0.01f;
    float far = 100.f;
    float fov = glm::radians(60.0f);

    app.m_mvpUniforms.projectionMatrix = glm::perspective(fov, ratio, near, far);
    app.m_mvpUniforms.modelMatrix = glm::mat4(1.0f);
    app.m_mvpUniforms.model2Matrix = glm::mat4(1.0f);

    constexpr std::chrono::milliseconds fixedTimeStep(50);

    // Initialize variables for tracking time
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto previousTime = currentTime;
    std::chrono::milliseconds lag(0);

//    physicManager.fixedUpdateGPU();

    while (app.isRunning()) {

        currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - previousTime);
        previousTime = currentTime;

        // Accumulate lag
        lag += elapsedTime;

        //////    FIXED UPDATE    //////
        while (lag >= fixedTimeStep) {
            // Perform fixed update tasks here
            physicManager.fixedUpdateGPU();

            // Decrease lag by the fixed time step
            lag -= fixedTimeStep;
        }
        auto t = static_cast<float>(glfwGetTime()); // glfwGetTime returns a double
        transformVertex(app, t);
        transformVertex2(app, t);
        app.onFrame();
    }

    app.onFinish();

    return 0;
}
