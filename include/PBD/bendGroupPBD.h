//
// Created by migue on 15/01/2024.
//

#ifndef WGPU_PS_BENDGROUPPBD_H
#define WGPU_PS_BENDGROUPPBD_H

#include <PBD/nodePBD.h>

class BendGroup {
public:
    std::vector<NodePBD*> nodes;
    NodePBD &x0, x1; //Internal nodes
    NodePBD &x2, x3; //External nodes
    MatrixXR Q;
    MatrixXR W;

    BendGroup(NodePBD &x0, NodePBD &x1, NodePBD &x2, NodePBD &x3) : x0(x0), x1(x1), x2(x2), x3(x3) {
        nodes.push_back(&x0);
        nodes.push_back(&x1);
        nodes.push_back(&x2);
        nodes.push_back(&x3);
    }

    void initialize() {
        //We compute the edges
        Vector3R e0 = x1.pos - x0.pos;
        Vector3R e1 = x2.pos - x1.pos;
        Vector3R e2 = x0.pos - x2.pos;
        Vector3R e3 = x3.pos - x0.pos;
        Vector3R e4 = x1.pos - x3.pos;

        //We compute the areas
        float A0 = area(x2.pos, x0.pos, x1.pos);
        float A1 = area(x3.pos, x0.pos, x1.pos);

        //We fill the K vector
        VectorXR K(4);
        K[0] = cotan(e0, e1) + cotan(e0, e4);
        K[1] = cotan(e0, e2) + cotan(e0, e3);
        K[2] = -cotan(e0, e1) - cotan(e0, e2);
        K[2] = -cotan(e0, e3) - cotan(e0, e4);

        Q = 3 * K * K.transpose() / (A0 + A1);

        W = MatrixXR::Identity(12, 12);
        for (int i = 0; i < 4; i++) {
            W.block<3, 3>(i * 3, i * 3) *= nodes[i]->massInv;
        }
    }

    void projectBendingConstraint(VectorXR &p) {
        float C = computeConstraintValue(p);
        VectorXR gradC = computeConstraintGradiant(p);
        float lagMult = C / (W * gradC).dot(gradC);
        VectorXR deltaX = (W * -lagMult) * gradC;

        for (int i = 0; i < 4; i++) {
            p.segment<3>(nodes[i]->index) += deltaX.segment<3>(i * 3);
        }
    }

private:
    static float cotan(Vector3R &a, Vector3R &b) {
        float modA = a.norm();
        float modB = b.norm();
        float cos = a.dot(b) / (modA * modB);
        float sin = a.cross(b).norm() / (modA * modB);

        return cos / sin;
    }

    static float area(Vector3R &a, Vector3R &b, Vector3R &c) {
        Vector3R AB = b - a;
        Vector3R AC = c - a;

        return AB.cross(AC).norm() * 0.5f;
    }

    float computeConstraintValue(VectorXR &p) {
        float sum = 0;

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                sum += Q(i, j) * p.segment<3>(nodes[i]->index).dot(p.segment<3>(nodes[j]->index));
            }
        }

        return 0.5f * sum;
    }

    VectorXR computeConstraintGradiant(VectorXR &p) {
        VectorXR grad(12);

        for (int i = 0; i < 4; i++) {
            Vector3R sum;
            for (int j = 0; j < 4; j++) {
                sum += Q(i, j) * p.segment<3>(nodes[j]->index);
            }
            grad.segment<3>(i * 3) = sum;
        }

        return grad;
    }
};

#endif //WGPU_PS_BENDGROUPPBD_H
