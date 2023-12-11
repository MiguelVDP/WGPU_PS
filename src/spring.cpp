#include <spring.h>

Spring::Spring(Node &nodeA, Node &nodeB, SpringType springType, PhysicManager &manager) : nodeA(nodeA), nodeB(nodeB),
                                                                                          springType(springType),
                                                                                          manager(manager) {
    direction = (nodeA.pos - nodeB.pos);
    length = direction.norm();
    length0 = length;
    direction.normalize();
}
void Spring::initialize(float stiff, float damp) {
    stiffness = stiff;
    damping = damp;
}

void Spring::updateState() {
    direction = nodeA.pos - nodeB.pos;
    length = direction.norm(); //norm() return the magnitude fo the vector
    direction.normalize();
}

void Spring::getForces(VectorXR& force) {

    static_cast<void>(force);
    Vector3R dirN = direction.normalized();
    Vector3R dampForce = -damping * dirN * dirN.dot(nodeA.pos - nodeB.pos);
    Vector3R totalForce = -stiffness * (length - length0) * dirN + dampForce;

    force.segment<3>(nodeA.index) += totalForce;
    force.segment<3>(nodeB.index) -= totalForce;

}

void Spring::getForceJacobians(MatrixXR& dFdx, MatrixXR& dFdv) {

    static_cast<void>(dFdx);
    static_cast<void>(dFdv);

}


