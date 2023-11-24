#ifndef WGPU_PS_SPRING_H
#define WGPU_PS_SPRING_H

#include <enums.h>
#include <Eigen/Dense>
#include <node.h>

class Spring{
public:
    float stiffness;
    float damping;

    Node &nodeA;
    Node &nodeB;

    SpringType springType;

    float length0;
    float length;
    VectorXR direction;

    Spring(Node &nodeA, Node &nodeB, SpringType springType, PhysicManager &manager);

    void initialize(float stiff, float damp);

    void updateState();

    void getForces(VectorXR force);

    void getForceJacobians(MatrixXR dFdx, MatrixXR dFdv);

private:
    PhysicManager &manager;

};

#endif //WGPU_PS_SPRING_H
