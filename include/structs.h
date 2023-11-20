#ifndef STRUCTS_H
#define STRUCTS_H

#include <glm/glm.hpp>

struct MyUniforms {
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;
    glm::mat4 model2Matrix;
};

struct CameraState{
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;
};

#endif