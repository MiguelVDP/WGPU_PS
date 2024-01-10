#ifndef WGPU_PS_PHYSICMANAGERPBD_H
#define WGPU_PS_PHYSICMANAGERPBD_H

#include <Eigen/Dense>
#include <iostream>
#include <PBD/simulablePBD.h>
#include <enums.h>
#include <memory>

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using MatrixXR = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3R = Eigen::Matrix<float, 3, 1>;


class PhysicManagerPBD{
public:
    bool paused;
    float timeStep;
    Vector3R gravity;
    std::vector<std::unique_ptr<SimulablePBD>> simObjs;
    int numDoFs;

    PhysicManagerPBD(){
        paused = true;
        timeStep = 0.005f;
        gravity = Vector3R(0.0f, -9.8f, 0.0f);
        numDoFs = 0;
    }

    void initialize();

    void fixedUpdate();

    void unPause();
};



#endif //WGPU_PS_PHYSICMANAGERPBD_H