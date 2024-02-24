//
// Created by Miguel on 30/12/2023.
//
#include <PBD/physicmanagerPBD.h>

void PhysicManagerPBD::initialize() {
    numDoFs = 0;

    for (auto &simObj: simObjs) {
        simObj->initialize(numDoFs);
        numDoFs += simObj->getNumDoFs();
    }
}

void PhysicManagerPBD::fixedUpdate() {
    if (paused) return;

    VectorXR x(numDoFs); //Actual position vector
    VectorXR p(numDoFs); //Predicted position vector
    VectorXR v(numDoFs); //Velocity vector
    VectorXR fExt(numDoFs); //External forces vector
    fExt.setZero();

    for (auto &sim: simObjs) {
        sim->getPosition(x);
        sim->getVelocity(v);
        sim->getExtFore(fExt);
    }

    //Compute velocity according to external forces
    v = v + timeStep * fExt;
    //Compute the predicted positions (without constraints)
    p = x + timeStep * v;

    //TODO Collisions

    //Apply constraints
    for(int it = 0 ; it < simIterations; it++){
        for (auto &sim: simObjs) {
            sim->projectConstraints(p);
        }
    }

    //Correct velocities
    v = (p - x) / timeStep;

//    std::cout << "----------------------------- \n Vf:" << std::endl;
//    for (int i = 0; i < p.size(); i += 3) {
//        std::cout << "(" << v[i] << ", " << v[i + 1] << ", " << v[i + 2] << ")" << std::endl;
//    }

    for (auto &sim: simObjs) {
        sim->setVelocity(v);
        sim->setPosition(p);
    }

    for (auto &sim: simObjs) {
        sim->updateObjectState();
    }

//    std::cout << "///////////////////////////////////" << std::endl;
}

void PhysicManagerPBD::unPause() {
    paused = !paused;
}

void PhysicManagerPBD::step() {
    paused = false;
    fixedUpdate();
    std::cout << "step" << std::endl;
    paused = true;
}

void PhysicManagerPBD::fixedUpdateGPU() {
    if (paused) return;

    VectorXR x(numDoFs); //Actual position vector
    VectorXR p(numDoFs); //Predicted position vector
    VectorXR v(numDoFs); //Velocity vector
    VectorXR fExt(numDoFs); //External forces vector
    std::vector<Vector32i> stretchCG;
    std::vector<VectorXR> stretchCGData;
    fExt.setZero();

    for (auto &sim: simObjs) {
        sim->getPosition(x);
        sim->getVelocity(v);
        sim->getExtFore(fExt);
        sim->getStretchColorGraph(stretchCG);
        sim->getStretchConstraintData(stretchCGData);
    }

    //Compute velocity according to external forces
    v = v + timeStep * fExt;
    //Compute the predicted positions (without constraints)
    p = x + timeStep * v;

    //TODO Collisions

    //Apply constraints
    //Stretch constraint
    for (int i = 0; i < simIterations; ++i) {
        app.onCompute(p, stretchCG, stretchCGData, 2);
    }

    //Correct velocities
    v = (p - x) / timeStep;

//    std::cout << "----------------------------- \n Pout:" << std::endl;
//    for (int i = 0; i < p.size(); i += 3) {
//        std::cout << "(" << p[i] << ", " << p[i + 1] << ", " << p[i + 2] << ")" << std::endl;
//    }

    for (auto &sim: simObjs) {
        sim->setVelocity(v);
        sim->setPosition(p);
    }

    for (auto &sim: simObjs) {
        sim->updateObjectState();
    }
}


