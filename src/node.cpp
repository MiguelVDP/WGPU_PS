//
// Created by migue on 17/11/2023.
//
#include "node.h"

#include <utility>

void Node::initialize(int idx, float m, float damp) {
    index = idx;
//    manager = physicManager;
    mass = m;
    damping = damp;
}

Node::Node(PhysicManager &man, Vector3R p) : manager(man) {
    pos = std::move(p);
    vel.setZero();
}

void Node::getPosition(VectorXR position) {
    position.setZero();

}

void Node::setPosition(VectorXR position) {

    position.setZero();
}


void Node::getVelocity(VectorXR velocity) {

    velocity.setZero();

}

void Node::setVelocity(VectorXR velocity) {

    velocity.setZero();
}

void Node::getForce(VectorXR force) {

    force.setZero();
}

void Node::getForceJacobian(VectorXR dFdx, VectorXR dFdv) {

    dFdv.setZero();
    dFdx.setZero();

}

void Node::getMass(VectorXR m) {

    m.setZero();

}

void Node::getMassInverse(VectorXR massInv) {

    massInv.setZero();

}
