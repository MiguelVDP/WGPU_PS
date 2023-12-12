//
// Created by migue on 17/11/2023.
//
#include "node.h"

#include <utility>

void Node::initialize(int idx, float m, float damp) {
    index = idx;
    mass = m;
    damping = damp;
}

Node::Node(PhysicManager &man, Vector3R p) : manager(man) {
    pos = std::move(p);
    vel.setZero();
}

void Node::getPosition(VectorXR &position) {
    position.segment<3>(index) = pos;
}

void Node::setPosition(VectorXR &position) {
    if(index == 0)
        return;

    pos = position.segment<3>(index);
}


void Node::getVelocity(VectorXR &velocity) {

    velocity.segment<3>(index) = vel;

}

void Node::setVelocity(VectorXR &velocity) {

    vel = velocity.segment<3>(index);
}

void Node::getForce(VectorXR &force) {

    //Gravity force
    Vector3R gForce(manager.gravity.x() * mass,
                    manager.gravity.y() * mass,
                    manager.gravity.z() * mass);
    force.segment<3>(index) += gForce;

    //Damping force
    Vector3R dForce(vel.x() * damping,
                    vel.y() * damping,
                    vel.z() * damping);
    force.segment<3>(index) -= dForce;
}

void Node::getForceJacobian(MatrixXR &dFdx, MatrixXR &dFdv) {

    //dFdx stays unchanged because the node force (gravity) does not depend on its position.
    static_cast<void>(dFdx); //We cast it to void to avoid the "unused parameter" error

    MatrixXR dampingMat = MatrixXR::Identity(3, 3) * -damping;
    dFdv.block<3, 3>(index, index) += dampingMat;
}

void Node::getMass(MatrixXR &m) {

    MatrixXR mInv = MatrixXR::Identity(3, 3) * mass;
    m.block<3, 3>(index, index) = mInv;

}

void Node::getMassInverse(MatrixXR &massInv) {

    MatrixXR mInv = MatrixXR::Identity(3, 3) * (1.0f / mass);
    massInv.block<3, 3>(index, index) = mInv;

}
