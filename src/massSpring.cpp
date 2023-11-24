#include <massSpring.h>

void MassSpring::initialize(int i) {

    fillNodesAndSprings();
    index = i;
    float nodeMass = mass / (float) nodes.size();
    for (int n = 0; n < (int) nodes.size(); n++) {
        nodes[n].initialize(index + 3 * i, nodeMass, dampingAlpha * nodeMass);
        //Lo de los fixers
    }

    for (auto spring: springs) {
        if (spring.springType == SpringType::Stretch)
            spring.initialize(stiffnessStretch, dampingBeta * stiffnessStretch);
        else if (spring.springType == SpringType::Bend)
            spring.initialize(stiffnessBend, dampingBeta * stiffnessBend);
    }
}

void MassSpring::fillNodesAndSprings() {

    //Generate all the nodes (one per vertex)
    for (int i = 0; i < object.positions.size(); i += 3) {
        Vector3R pos(object.positions[i],
                     object.positions[i + 1],
                     object.positions[i + 2]);

        nodes.emplace_back(manager, pos);
    }

    //Read the mesh edges to create the cloth springs.
    std::map<Edge, int, EdgeComparer> edgeMap;
    for (int i = 0; i < object.triangles.size(); i += 3) {
        for (int j = 0; j < 3; j++) {
            int a = i + j;
            int b = i + ((j + 1) % 3);
            int o = i + ((j + 2) % 3);

            Edge edge(object.triangles[a], object.triangles[b], object.triangles[o]);
            if (edgeMap.insert({edge, i}).second) {
                //If the edge already exist we should create a bend spring
                auto it = edgeMap.find(edge);
                springs.emplace_back(nodes[edge.o], nodes[it->first.o], SpringType::Bend, manager);
            }
        }
    }
    //Once all the edges have been created we just have to create the stretch springs
    for (auto &it: edgeMap) {
        springs.emplace_back(nodes[it.first.a], nodes[it.first.b], SpringType::Stretch, manager);
    }
}

int MassSpring::getNumDoFs() {
    return 3 * (int) nodes.size();
}

void MassSpring::getPosition(VectorXR position) {
    for (auto node: nodes)
        node.getPosition(position);
}

void MassSpring::setPosition(VectorXR position) {

    for (auto node: nodes)
        node.setPosition(position);

    for (auto spring: springs)
        spring.updateState();
}

void MassSpring::getVelocity(VectorXR velocity) {

    for (auto node: nodes)
        node.getVelocity(velocity);
}

void MassSpring::setVelocity(VectorXR velocity) {

    for (auto node: nodes)
        node.setVelocity(velocity);
}

void MassSpring::getFore(VectorXR force) {

    for (auto node: nodes)
        node.getForce(force);

    for (auto spring: springs)
        spring.getForces(force);
}

void MassSpring::getForceJacobian(MatrixXR dFdx, MatrixXR dFdv) {

    for (auto node: nodes)
        node.getForceJacobian(dFdx, dFdv);

    for (auto spring: springs)
        spring.getForceJacobians(dFdx, dFdv);
}

void MassSpring::getMass(MatrixXR m) {

    for (auto node: nodes)
        node.getMass(m);
}

void MassSpring::getMassInverse(MatrixXR massInv) {
    for (auto node: nodes)
        node.getMassInverse(massInv);
}

MassSpring::MassSpring(PhysicManager &manager, Object &object) : manager(manager), object(object) {}

