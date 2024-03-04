#ifndef STRUCTS_H
#define STRUCTS_H

#include <glm/glm.hpp>
#include <map>
#include <functional>
#include <webgpu/webgpu.hpp>

struct MyUniforms {
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;
    glm::mat4 model2Matrix;
};

struct CameraState {
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;
};

struct StepData{
    uint32_t num_dof;
    float time_step;
};


enum ConstraintType{
    STRETCH,
    BEND
};

enum ComputeOperation{
    COMPUTE_P,
    COMPUTE_V,
    PROJECT_STRETCH,
};

struct SimulationStepStruct{
    wgpu::ComputePipeline pipeline;
    wgpu::BindGroup bindGroup;
};

struct Edge {
    int a;
    int b;
    int o;

    // Define the hash function for the structure
    std::size_t operator()(const Edge &e) const {
        // Use a combination of hash values for each member
        if (e.a > e.b)
            return e.a * 31 + e.b;

        return e.b * 31 + e.a;

    }

    // Define the equality operator
    bool operator==(const Edge &e2) const {
        return ((this->a == e2.a && this->b == e2.b) || (this->a == e2.b && this->b == e2.a));
    }
};

#endif