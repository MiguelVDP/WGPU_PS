//
// Created by migue on 09/01/2024.
//

#ifndef WGPU_PS_NODEPBD_H
#define WGPU_PS_NODEPBD_H

#include <Eigen/Dense>
#include <utility>
#include <PBD/physicmanagerPBD.h>

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using MatrixXR = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3R = Eigen::Matrix<float, 3, 1>;

class NodePBD{
public:
    float mass;
    float massInv;
    bool fixed;
    int index;
    Vector3R pos;
    Vector3R vel;

    explicit NodePBD(PhysicManagerPBD &manager, Vector3R p);

    void initialize(int idx, float m);

    void getPosition(VectorXR& position);

    void setPosition(VectorXR& position);

    void getVelocity(VectorXR& velocity);

    void setVelocity(VectorXR& velocity);

    void getMass(MatrixXR & m);

    void getMassInverse(MatrixXR& mInv);

    void getExtForce(VectorXR& force);

private:
    const PhysicManagerPBD &manager;
};

#endif //WGPU_PS_NODEPBD_H
