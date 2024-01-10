//
// Created by migue on 09/01/2024.
//
#include <PBD/nodePBD.h>

NodePBD::NodePBD(PhysicManagerPBD &man, Vector3R p) : manager(man) {
    pos = p;
    vel.setZero();
}

void NodePBD::initialize(int idx, float m) {
    index = idx;
    mass = m;
    massInv = 1.0f / m;
}

void NodePBD::getPosition(VectorXR &position) {
    position.segment<3>(index) = pos;
}

void NodePBD::setPosition(VectorXR &position) {
    pos = position.segment<3>(index);
}

void NodePBD::getVelocity(VectorXR &velocity) {
    velocity.segment<3>(index) = pos;
}

void NodePBD::setVelocity(VectorXR &velocity) {
    vel = velocity.segment<3>(index);
}

void NodePBD::getMass(MatrixXR &m) {
    MatrixXR mMat = MatrixXR::Identity(3, 3) * mass;
    m.block<3, 3>(index, index) = mMat;
}

void NodePBD::getMassInverse(MatrixXR &mInv) {
    MatrixXR mInvMat = MatrixXR::Identity(3, 3) * massInv;
    mInv.block<3, 3>(index, index) = mInvMat;
}

void NodePBD::getExtForce(VectorXR &extForce) {
    //Gravity force
    Vector3R gForce(manager.gravity.x() * mass,
                    manager.gravity.y() * mass,
                    manager.gravity.z() * mass);
    extForce.segment<3>(index) += gForce;
}
