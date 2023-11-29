#ifndef PHYSIC_MANAGER_H
#define PHYSIC_MANAGER_H

#include <Eigen/Dense>
#include <iostream>
#include <massSpring.h>
#include <enums.h>
#include <memory>

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using MatrixXR = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3R = Eigen::Matrix<float, 3, 1>;

class MassSpring;

class PhysicManager{

public:
    bool paused;
    float timeStep;
    Vector3R gravity;
    std::vector<MassSpring> simObjs;
    Integration integrationMethod;
    int numDoFs;

    PhysicManager();

    void initialize();

    void fixedUpdate();

    void stepSymplectic();

    void unPause();

};

#endif