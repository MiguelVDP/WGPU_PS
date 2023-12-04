#include <massSpring.h>

void MassSpring::initialize(int idx) {

    fillNodesAndSprings();
    index = idx;
    float nodeMass = mass / (float) nodes.size();
    for (int i = 0; i < (int)nodes.size(); i++) {
        nodes[i].initialize(index + 3 * i, nodeMass, dampingAlpha * nodeMass);
        //Lo de los fixers
    }

    for (Spring& spring: springs) {
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
    std::cout << "Nodes: " << nodes.size() << std::endl;
    std::cout << "Vertices: " << object.positions.size() << std::endl;
    std::cout << "Indices: " << object.triangles.size() << std::endl;

    //Read the mesh edges to create the cloth springs.
    std::map<Edge, int, EdgeComparer> edgeMap;
    for (int i = 0; i < object.triangles.size(); i += 3) {
        for (int j = 0; j < 3; j++) {
            int a = i + j;
            int b = i + ((j + 1) % 3);
            int o = i + ((j + 2) % 3);

            Edge edge((int)object.triangles[a], (int)object.triangles[b], (int)object.triangles[o]);
            if (!edgeMap.insert({edge, i}).second) {
                //If the edge already exist we should create a bend spring
                auto it = edgeMap.find(edge);
                Edge aux = it->first;
                if(it == edgeMap.end()){
                    for(auto e:edgeMap){
                        if((e.first.a == edge.a && e.first.b == edge.b)||(e.first.a == edge.b && e.first.b == edge.a)){
                            aux = e.first;
                            break;
                        }
                    }
                }
                springs.emplace_back(nodes[edge.o], nodes[aux.o], SpringType::Bend, manager);
            }
        }
    }
    //Once all the edges have been created we just have to create the stretch springs
    for (auto &it: edgeMap) {
        springs.emplace_back(nodes[it.first.a], nodes[it.first.b], SpringType::Stretch, manager);
    }
}

MassSpring::MassSpring(float mass, float stiffnessStretch, float stiffnessBend, float dampingAlpha, float dampingBeta,
                       PhysicManager &manager, Object &object) : mass(mass), stiffnessStretch(stiffnessStretch),
                                                                 stiffnessBend(stiffnessBend),
                                                                 dampingAlpha(dampingAlpha), dampingBeta(dampingBeta),
                                                                 manager(manager), object(object) {}

int MassSpring::getNumDoFs() {
    return 3 * (int) nodes.size();
}

void MassSpring::getPosition(VectorXR& position) {
    for (Node& node: nodes)
        node.getPosition(position);
}

void MassSpring::setPosition(VectorXR& position) {

    for (Node& node: nodes)
        node.setPosition(position);

    for (Spring& spring: springs)
        spring.updateState();
}

void MassSpring::getVelocity(VectorXR& velocity) {

    for (Node& node: nodes)
        node.getVelocity(velocity);
}

void MassSpring::setVelocity(VectorXR& velocity) {

    for (Node& node: nodes)
        node.setVelocity(velocity);
}

void MassSpring::getFore(VectorXR& force) {

    for (Node& node: nodes)
        node.getForce(force);

//    for (Spring& spring: springs)
//        spring.getForces(force);
}

void MassSpring::getForceJacobian(MatrixXR& dFdx, MatrixXR& dFdv) {

    for (Node& node: nodes)
        node.getForceJacobian(dFdx, dFdv);

    for (Spring& spring: springs)
        spring.getForceJacobians(dFdx, dFdv);
}

void MassSpring::getMass(MatrixXR & m) {

    for (Node& node: nodes)
        node.getMass(m);
}

void MassSpring::getMassInverse(MatrixXR& massInv) {

    for (Node& node: nodes) {
        node.getMassInverse(massInv);
    }
}

MassSpring::MassSpring(PhysicManager &manager, Object &object) : manager(manager), object(object) {}

void MassSpring::updateObjectState() {
    getPosition(object.positions);
}


