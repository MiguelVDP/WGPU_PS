//
// Created by migue on 10/01/2024.
//
#include "PBD/massSpringPBD.h"

MassSpringPBD::MassSpringPBD(float mass, float stiffnessStretch, float stiffnessBend, PhysicManagerPBD &manager,
                             Object &object) : mass(mass), stiffnessStretch(stiffnessStretch),
                                               stiffnessBend(stiffnessBend), manager(manager), object(object) {}

void MassSpringPBD::initialize(int idx) {
    fillNodesAndSprings();
    index = idx;
    float nodeMass = mass / (float) nodes.size();
    for (int i = 0; i < (int)nodes.size(); i++) {
        nodes[i].initialize(index + 3 * i, nodeMass);
        //Lo de los fixers
    }

    for (SpringPBD& spring: springs) {

            spring.initialize(stiffnessStretch);
    }
}

void MassSpringPBD::fillNodesAndSprings() {
    //Generate all the nodes (one per vertex)
    for (int i = 0; i < object.positions.size(); i += 3) {
        Vector3R pos(object.positions[i],
                     object.positions[i + 1],
                     object.positions[i + 2]);

        nodes.emplace_back(manager, pos);
    }
    std::cout << "Nodes: " << nodes.size() << std::endl;
    std::cout << "Vertices: " << object.positions.size() << std::endl;
    std::cout << "Indices: " << object.triangles.size() << std::endl;

    //Read the mesh edges to create the cloth springs.
    std::unordered_set<Edge, Edge> edgeSet;
    int bCount = 0;
    for (int i = 0; i < object.triangles.size(); i += 3) {
        for (int j = 0; j < 3; j++) {
            int a = i + j;
            int b = i + ((j + 1) % 3);
            int o = i + ((j + 2) % 3);

            Edge edge = {(int)object.triangles[a], (int)object.triangles[b], (int)object.triangles[o]};
            auto it = edgeSet.insert(edge);
            if (!it.second) {
                bCount++;
                //If the edge already exist we should create a bend spring
                springs.emplace_back(nodes[edge.o], nodes[it.first->o]);
            }
        }
    }
    std::cout << "Stretch springs: " << edgeSet.size() << " Bend Springs: " << bCount << std::endl;
    //Once all the edges have been created we just have to create the stretch springs
    for (auto &it: edgeSet) {
        springs.emplace_back(nodes[it.a], nodes[it.b]);
    }
}

void MassSpringPBD::getPosition(VectorXR &position) {
    for(NodePBD& node : nodes){
        node.getPosition(position);
    }
}

void MassSpringPBD::setPosition(VectorXR &position) {
    for(NodePBD& node : nodes){
        node.setPosition(position);
    }
}

void MassSpringPBD::getVelocity(VectorXR &velocity) {
    for(NodePBD& node : nodes){
        node.getVelocity(velocity);
    }
}

void MassSpringPBD::setVelocity(VectorXR &velocity) {
    for(NodePBD& node : nodes){
        node.setVelocity(velocity);
    }
}

void MassSpringPBD::getExtFore(VectorXR &force) {
    for(NodePBD& node : nodes){
        node.getExtForce(force);
    }
}

void MassSpringPBD::getMass(MatrixXR &m) {
    for(NodePBD& node : nodes){
        node.getMass(m);
    }
}

void MassSpringPBD::getMassInverse(MatrixXR &massInv) {
    for(NodePBD& node : nodes){
        node.getMassInverse(massInv);
    }
}

void MassSpringPBD::updateObjectState() {
    getPosition(object.positions);
    object.computeNormals();
}
