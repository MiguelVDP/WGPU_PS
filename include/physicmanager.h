#ifndef PHYSIC_MANAGER_H
#define PHYSIC_MANAGER_H

#include <Eigen/Dense>
#include <simulable.h>
#include <enums.h>

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using MatrixXR = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3R = Eigen::Matrix<float, 3, 1>;


class PhysicManager{

public:
    bool paused;
    float timeStep;
    Vector3R gravity;
    std::vector<Simulable> simObjs;
    Integration IntegrationMethod;
    int numDoFs;

    void initialize();


};

#endif