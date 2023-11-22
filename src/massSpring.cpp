#include <massSpring.h>

void MassSpring::Initialize(int i) {

}

void MassSpring::fillNodesAndSprings() {

    //Generate all the nodes (one per vertex)
    for (int i = 0; i < object.positions.size(); i += 3) {
        Vector3R pos(object.positions[i],
                     object.positions[i + 1],
                     object.positions[i + 2]);
        Node n(manager, pos);
        nodes.push_back(n);
    }

    //Read the mesh edges to create the cloth springs.
    std::map<Edge, int, EdgeComparer> edgeMap;
    for (int i = 0; i < object.triangles.size(); i += 3) {
        for (int j = 0; j < 3; j++) {
            int a = i + j;
            int b = i + ((j + 1) % 3);
            int o = i + ((j + 2) % 3);

            Edge edge(object.triangles[a], object.triangles[b], object.triangles[o]);
            if(edgeMap.insert({edge, i}).second){
                //If the edge already exist we should create a bend spring
                auto it = edgeMap.find(edge);
                Spring bendS(nodes[edge.o], nodes[it->first.o], SpringType::Bend, manager);
                springs.push_back(bendS);
            }
        }
    }
    //Once all the edges have been created we just have to create the stretch springs
    for(auto & it : edgeMap){
        Spring stretchS(nodes[it.first.a], nodes[it.first.b], SpringType::Stretch, manager);
        springs.push_back(stretchS);
    }
}

int MassSpring::getNumDoFs() {
    return 0;
}

void MassSpring::getPosition(VectorXR position) {

}

void MassSpring::setPosition(VectorXR position) {

}

void MassSpring::getVelocity(VectorXR velocity) {

}

void MassSpring::setVelocity(VectorXR velocity) {

}

void MassSpring::getFore(VectorXR force) {

}

void MassSpring::getForceJacobian(MatrixXR dFdx, MatrixXR dFdv) {

}

void MassSpring::getMass(MatrixXR m) {

}

void MassSpring::getMassInverse(MatrixXR massInv) {

}

MassSpring::MassSpring(PhysicManager &manager, Object &object) : manager(manager), object(object) {}

