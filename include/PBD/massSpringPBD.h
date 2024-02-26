//
// Created by migue on 09/01/2024.
//

#ifndef WGPU_PS_MASSSPRINGPBD_H
#define WGPU_PS_MASSSPRINGPBD_H

#include <PBD/springPBD.h>
#include <PBD/bendGroupPBD.h>
#include <PBD/simulablePBD.h>
#include <structs.h>
#include <object.h>
#include <unordered_set>

class MassSpringPBD : public SimulablePBD {

    std::vector<NodePBD> nodes;
    std::vector<SpringPBD> springs;
    std::vector<BendGroup> bendingGroups;
    std::vector<Vector32i> stretchColorGraph;
    std::vector<VectorXR> stretchColorGraphData;
//    Vector32i stretchStencils;

    float mass{};
    float stiffnessStretch{};
    float stiffnessBend{};
    int index{};

public:
    MassSpringPBD(float mass, float stiffnessStretch, float stiffnessBend, PhysicManagerPBD &manager, Object &object);

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

    void projectConstraints(VectorXR &p) override;

//    void getStretchStencilIdx(Vector32i &stIdx) override;

    void getStretchConstraintData(std::vector<VectorXR> &data) override;

    void getStretchColorGraph(std::vector<Vector32i> &cg) override;

    void getStretchColorCount(int &color_count) override;

private:

    ~MassSpringPBD() override = default;

    PhysicManagerPBD &manager;
    Object &object;

    void updateObjectState() override;

    void fillNodesAndSprings();

    void fillStretchColorGraph();

};

#endif //WGPU_PS_MASSSPRINGPBD_H
