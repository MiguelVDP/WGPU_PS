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
    MatrixXR massInv(numDoFs, numDoFs);
    massInv.setZero();

    for (auto &sim: simObjs) {
        sim->getPosition(x);
        sim->getVelocity(v);
        sim->getExtFore(fExt);
        sim->getMassInverse(massInv);
    }

    v = v + timeStep * fExt;
    p = x + timeStep * v;



}

void PhysicManagerPBD::unPause() {

}


