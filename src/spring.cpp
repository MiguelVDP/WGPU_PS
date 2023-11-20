#include <spring.h>

Spring::Spring(Node &nodeA, Node &nodeB, SpringType springType, PhysicManager &manager) : nodeA(nodeA), nodeB(nodeB),
                                                                                          springType(springType),
                                                                                          manager(manager) {}
void Spring::initialize(float stiff, float damp) {
    stiffness = stiff;
    damping = damp;
}

void Spring::updateState() {

}

void Spring::getForces(VectorXR force) {

    force.setZero();

}

void Spring::getForceJacobians(MatrixXR dFdx, MatrixXR dFdv) {

    dFdx.setZero();
    dFdv.setZero();

}


