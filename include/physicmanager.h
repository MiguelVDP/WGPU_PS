#ifndef PHYSIC_MANAGER_H
#define PHYSIC_MANAGER_H

#include <Eigen/Dense>
#include <iostream>
#include <simulable.h>
#include <enums.h>
#include <memory>

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using MatrixXR = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3R = Eigen::Matrix<float, 3, 1>;


class PhysicManager{

public:
    bool paused;
    float timeStep;
    Vector3R gravity;
    std::vector<std::unique_ptr<Simulable>> simObjs;
    Integration integrationMethod;
    int numDoFs;

    PhysicManager();

    void initialize();

    void fixedUpdate();

    void stepSymplectic();

    void unPause();

};

#endif