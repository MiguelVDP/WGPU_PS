//
// Created by migue on 09/01/2024.
//

#ifndef WGPU_PS_SPRINGPBD_H
#define WGPU_PS_SPRINGPBD_H

#include <PBD/nodePBD.h>

class SpringPBD {
public:
    NodePBD &nodeA;
    NodePBD &nodeB;
    float length0;
    float stiffness;
    VectorXR direction;


    SpringPBD(NodePBD &node1, NodePBD &node2) : nodeA(node1), nodeB(node2) {
        direction = (nodeA.pos - nodeB.pos);
        length0 = direction.norm();
        direction.normalize();
    }

    void initialize(float stiff) {
        stiffness = stiff;
    }

    void projectDistanceConstraint(VectorXR &p) {
        Vector3R pA = p.segment<3>(nodeA.index);
        Vector3R pB = p.segment<3>(nodeB.index);
        float wA = nodeA.massInv;
        float wB = nodeB.massInv;

        Vector3R dist = pA - pB;

        float length = dist.norm();
        dist.normalize();

        Vector3R correction = ((length - length0) * dist) / (wA + wB);

        pA -= wA * correction;
        pB += wB * correction;

        p.segment<3>(nodeA.index) = pA;
        p.segment<3>(nodeB.index) = pB;
    }

};

#endif //WGPU_PS_SPRINGPBD_H
