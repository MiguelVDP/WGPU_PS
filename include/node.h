#ifndef WGPU_PS_NODE_H
#define WGPU_PS_NODE_H

#include <Eigen/Dense>
#include <utility>
#include <physicmanager.h>

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using MatrixXR = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3R = Eigen::Matrix<float, 3, 1>;

class Node{
public:
    float mass;
    float damping;
    bool fixed;
    int index;
    Vector3R pos;
    Vector3R vel;

    explicit Node(PhysicManager &manager, Vector3R p);

    void initialize(int idx, float m, float damp);

    void getPosition(VectorXR& position);

    void setPosition(VectorXR& position);

    void getVelocity(VectorXR& velocity);

    void setVelocity(VectorXR& velocity);

    void getMass(MatrixXR & m);

    void getMassInverse(MatrixXR& massInv);

    void getForce(VectorXR& force);

    void getForceJacobian(MatrixXR& dFdx, MatrixXR& dFdv);

private:
    const PhysicManager &manager;

};

#endif //WGPU_PS_NODE_H
