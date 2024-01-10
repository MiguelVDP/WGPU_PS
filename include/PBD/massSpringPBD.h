//
// Created by migue on 09/01/2024.
//

#ifndef WGPU_PS_MASSSPRINGPBD_H
#define WGPU_PS_MASSSPRINGPBD_H

#include <PBD/simulablePBD.h>
#include <PBD/springPBD.h>
#include <structs.h>
#include <object.h>
#include <unordered_set>

class MassSpringPBD : public SimulablePBD {

    std::vector<NodePBD> nodes;
    std::vector<SpringPBD> springs;

    float mass{};
    float stiffnessStretch{};
    float stiffnessBend{};
    int index{};

public:
    MassSpringPBD(float mass, float stiffnessStretch, float stiffnessBend, PhysicManagerPBD &manager, Object &object);

private:

    void initialize(int i) override;

    int getNumDoFs() override {
        return 3 * (int) nodes.size();
    }

    void getPosition(VectorXR &position) override;

    void setPosition(VectorXR &position) override;

    void getVelocity(VectorXR &velocity) override;

    void setVelocity(VectorXR &velocity) override;

    void getExtFore(VectorXR &force) override;

    void getMass(MatrixXR &mass) override;

    void getMassInverse(MatrixXR &massInv) override;

    ~MassSpringPBD() override = default;

private:

    PhysicManagerPBD &manager;
    Object &object;

    void updateObjectState() override;

    void fillNodesAndSprings();

};

#endif //WGPU_PS_MASSSPRINGPBD_H
