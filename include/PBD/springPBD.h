//
// Created by migue on 09/01/2024.
//

#ifndef WGPU_PS_SPRINGPBD_H
#define WGPU_PS_SPRINGPBD_H

#include <PBD/nodePBD.h>

class SpringPBD{
public:
    NodePBD &nodeA;
    NodePBD &nodeB;
    float length;
    float length0;
    float stiffness;
    VectorXR direction;


    SpringPBD(NodePBD &node1, NodePBD &node2) : nodeA(node1), nodeB(node2){
        direction = (nodeA.pos - nodeB.pos);
        length = direction.norm();
        length0 = length;
        direction.normalize();
    }

    void initialize(float stiff){
        stiffness = stiff;
    }

};

#endif //WGPU_PS_SPRINGPBD_H
