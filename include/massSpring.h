#ifndef WGPU_PS_MASSSPRING_H
#define WGPU_PS_MASSSPRING_H

#include <spring.h>
#include <simulable.h>
#include <object.h>
#include <structs.h>
#include <map>

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using MatrixXR = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3R = Eigen::Matrix<float, 3, 1>;

class MassSpring : public Simulable{
public:

    std::vector<Node> nodes;
    std::vector<Spring> springs;

    float mass{};
    float stiffnessStretch{};
    float stiffnessBend{};
    float dampingAlpha{};
    float dampingBeta{};
    int index{};

    MassSpring(PhysicManager &manager, Object &object);

    MassSpring(float mass, float stiffnessStretch, float stiffnessBend, float dampingAlpha, float dampingBeta,
               PhysicManager &manager, Object &object);

    void initialize(int i) override;

    int getNumDoFs() override;

    void getPosition(VectorXR& position) override;

    void setPosition(VectorXR& position) override;

    void getVelocity(VectorXR& velocity) override;

    void setVelocity(VectorXR& velocity) override;

    void getFore(VectorXR& force) override;

    void getForceJacobian(MatrixXR& dFdx, MatrixXR& dFdv) override;

    void getMass(MatrixXR& m) override;

    void getMassInverse(MatrixXR& massInv) override;

    ~MassSpring() override = default;

private:

    void updateObjectState() override;

    PhysicManager &manager;
    Object &object;

    void fillNodesAndSprings();

};

#endif //WGPU_PS_MASSSPRING_H
