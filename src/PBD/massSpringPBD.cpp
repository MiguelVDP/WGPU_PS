//
// Created by migue on 10/01/2024.
//
#include <PBD/massSpringPBD.h>

MassSpringPBD::MassSpringPBD(float mass, float stiffnessStretch, float stiffnessBend, PhysicManagerPBD &manager,
                             Object &object) : mass(mass), stiffnessStretch(stiffnessStretch),
                                               stiffnessBend(stiffnessBend), manager(manager), object(object) {}

void MassSpringPBD::initialize(int idx) {
    fillNodesAndSprings();
    index = idx;
    float nodeMass = mass / (float) nodes.size();
    for (int i = 0; i < (int) nodes.size(); i++) {
        nodes[i].initialize(index + 3 * i, nodeMass);
        //Lo de los fixers
    }

    //We set the stretch Stencil vector size
//    stretchStencils.resize((int) springs.size() * 2);

    for (int i = 0; i < (int) springs.size(); i++) {
        springs[i].initialize(stiffnessStretch);
//        stretchStencils[i * 2] = springs[i].nodeA.index;
//        stretchStencils[i * 2 + 1] = springs[i].nodeB.index;
    }

    //
    for (BendGroup &bendGroup: bendingGroups) {
        bendGroup.initialize();
    }

    fillStretchColorGraph();
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

            Edge edge = {(int) object.triangles[a], (int) object.triangles[b], (int) object.triangles[o]};
            auto it = edgeSet.insert(edge);
            if (!it.second) {
                bCount++;
                //If the edge already exist we should create a bend spring
//                bendingGroups.emplace_back(nodes[edge.a], nodes[edge.b], nodes[edge.o], nodes[it.first->o]);
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
    for (NodePBD &node: nodes) {
        node.getPosition(position);
    }
}

void MassSpringPBD::setPosition(VectorXR &position) {
    for (NodePBD &node: nodes) {
        node.setPosition(position);
    }
}

void MassSpringPBD::getVelocity(VectorXR &velocity) {
    for (NodePBD &node: nodes) {
        node.getVelocity(velocity);
    }
}

void MassSpringPBD::setVelocity(VectorXR &velocity) {
    for (NodePBD &node: nodes) {
        node.setVelocity(velocity);
    }
}

void MassSpringPBD::getExtFore(VectorXR &force) {
    for (NodePBD &node: nodes) {
        node.getExtForce(force);
    }
}

void MassSpringPBD::getMass(MatrixXR &m) {
    for (NodePBD &node: nodes) {
        node.getMass(m);
    }
}

void MassSpringPBD::getMassInverse(MatrixXR &massInv) {
    for (NodePBD &node: nodes) {
        node.getMassInverse(massInv);
    }
}

void MassSpringPBD::updateObjectState() {
    getPosition(object.positions);
    object.computeNormals();
}

void MassSpringPBD::projectConstraints(VectorXR &p) {
    for (auto &spring: springs) {
        spring.projectDistanceConstraint(p);
    }

    for (auto &bendGroup: bendingGroups) {
        bendGroup.projectBendingConstraint(p);
    }
}

//void MassSpringPBD::getStretchStencilIdx(Vector32i &stIdx) {
//    int prevSize = (int) stIdx.size();
//    int stencilCount = (int) stretchStencils.size();
//
//    //Resize the vector
//    stIdx.conservativeResize(prevSize + stencilCount);
//
//    //Assign the new segment a value.
//    stIdx.tail(stencilCount) = stretchStencils;
//}


void MassSpringPBD::getStretchConstraintData(std::list<VectorXR> &data) {
    for(const auto& it : stretchColorGraphData)
        data.push_back(it);
}

void MassSpringPBD::getStretchColorGraph(std::list<Vector32i> &cg) {

    for(const auto& it : stretchColorGraph)
        cg.push_back(it);
}

void MassSpringPBD::fillStretchColorGraph() {
    const int stretchCount = int(springs.size());

    //Initialize the colorGraph
    int *result = new int[stretchCount];

    //Assign a first color to the first constraint
    result[0] = 0;
    int maxColor = 0;

    // Initialize remaining V-1 vertices as unassigned
    for (int u = 1; u < stretchCount; u++)
        result[u] = -1;  // no color is assigned to u

    // A temporary array to store the available colors. False
    // value of available[cr] would mean that the color cr is
    // assigned to one of its adjacent vertices
    bool *available = new bool[stretchCount];
    for (int cr = 0; cr < stretchCount; cr++)
        available[cr] = true;

    // Assign colors to remaining V-1 vertices
    for (int u = 1; u < stretchCount; u++) {
        SpringPBD spring = springs[u];

        // Process all adjacent vertices and flag their colors
        // as unavailable
        for (int i = 0; i < stretchCount; i++) {
            if(i == u) continue;    //We do not want to compare a spring with itself
            //We check for adjacency
            if(spring.checkAdjacency(springs[i])){
                //Check if adjacent constraints are unavailable
                if(result[i] != -1)
                    available[result[i]] = false;
            }
        }

        //Find the first available color
        int color;
        for(color = 0; color < stretchCount; color++)
            if(available[color]) break;

        result[u] = color; //Assign the color
        if(color > maxColor) maxColor = color;

        //reset the values back to true for the next iteration
        for (int cr = 0; cr < stretchCount; cr++)
            available[cr] = true;
    }

    int colorCount = maxColor + 1;
    stretchColorGraph.resize(colorCount);
    stretchColorGraphData.resize(colorCount);
    auto crData = stretchColorGraphData.begin();
    int color = 0;
    for(auto & cr : stretchColorGraph){
        for (int u = 0; u < stretchCount; u++)
            if(result[u] == color){
                cr.conservativeResize(cr.size() + 2);
                cr.tail(2) << springs[u].nodeA.index, springs[u].nodeB.index;
                crData->conservativeResize(crData->size() + 3);
                crData->tail(3) << springs[u].length0, springs[u].nodeA.massInv, springs[u].nodeB.massInv;
//                cr.push_back(springs[u].nodeA.index);
//                cr.push_back(springs[u].nodeB.index);
//                crData->push_back(springs[u].length0);
//                crData->push_back(springs[u].nodeA.massInv);
//                crData->push_back(springs[u].nodeB.massInv);
            }
        color++;
        crData++;
    }


//    // print the result
//    color = 0;
//    crData = stretchColorGraphData.begin();
//    for(auto & cr : stretchColorGraph) {
//        std::cout << "Color " << color << " (" << cr.size() << ")---> ";
//        for(auto id : cr)
//            std::cout << id << ", ";
//        std::cout << " ||  Data ---> ";
//        for(auto d : *crData)
//            std::cout << d << ", ";
//        std::cout << std::endl;
//        color++;
//        crData++;
//    }
//    std::cout << std::endl;

}
