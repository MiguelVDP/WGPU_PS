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

    for (auto &sim: simObjs) {
        sim->projectConstraints(p);
    }

    //Correct velocities
    v = (p - x) / timeStep;

    for (auto &sim: simObjs) {
        sim->setVelocity(v);
        sim->setPosition(p);
    }

}

void PhysicManagerPBD::unPause() {
    paused = !paused;
}


