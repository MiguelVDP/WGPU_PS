#include <physicmanager.h>

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using MatrixXR = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3R = Eigen::Matrix<float, 3, 1>;

void PhysicManager::initialize() {
    numDoFs = 0;

    for (auto simObj: simObjs) {
        simObj->initialize(numDoFs);
        numDoFs += simObj->getNumDoFs();
    }
}

PhysicManager::PhysicManager() {
    paused = true;
    timeStep = 0.1f;
    gravity = Vector3R(0.0f, -9.8f, 0.0f);
    integrationMethod = Integration::Symplectic;
}

void PhysicManager::fixedUpdate() {

    if (paused) return;

    switch (integrationMethod) {
        case Integration::Symplectic:
            stepSymplectic();
            break;
        case Integration::Explicit:
            std::cout << "Explicit method not implemented." << std::endl;
            break;
        case Integration::Implicit:
            std::cout << "Implicit method not implemented." << std::endl;
            break;
        default:
            std::cerr << "INTEGRATION METHOD NOT SPECIFIED!" << std::endl;
            break;
    }

    for (auto &sim: simObjs) {
        sim->updateObjectState();
    }
    paused = true;
}

void PhysicManager::stepSymplectic() {
    VectorXR x(numDoFs);
    VectorXR v(numDoFs);
    VectorXR f(numDoFs);
    f.setZero();
    MatrixXR massInv(numDoFs, numDoFs);
    massInv.setZero();

    for (auto &sim: simObjs) {
        sim->getPosition(x);
        sim->getVelocity(v);
        sim->getFore(f);
        sim->getMassInverse(massInv);
    }

    v += timeStep * (massInv * f);
    x += timeStep * v;

    for (auto &sim: simObjs) {
        sim->setPosition(x);
        sim->setVelocity(v);
    }
}

void PhysicManager::unPause() {
    paused = !paused;
}


