//
// Created by migue on 13/12/2023.
//
#include "object.h"


double computeCos(Vector3R a, Vector3R b, Vector3R c) {
    Vector3R ab = b - a;
    Vector3R ac = c - a;
    return ab.dot(ac) / (ab.norm() * ab.norm());
}

void Object::computeNormals() {
    int numTriangles = (int) triangles.size() / 3;
    int numVertex = (int) positions.size() / 3;
    std::vector<Vector3R> sumAreaXNorm(numVertex, Vector3R(0, 0, 0));
    std::vector<float> sumAreas(numVertex, 0);

    for (int i = 0; i < numTriangles; i++) {
        //We read the triangle
        uint16_t idxA = triangles[3 * i];
        uint16_t idxB = triangles[3 * i + 1];
        uint16_t idxC = triangles[3 * i + 2];

        Vector3R a = positions.segment<3>(3 * idxA);
        Vector3R b = positions.segment<3>(3 * idxB);
        Vector3R c = positions.segment<3>(3 * idxC);

        //We then compute its area and its normal
        Vector3R crossProd = (b - a).cross(c - a);
        float tArea = crossProd.norm() * 0.5f;
        Vector3R tNormal = crossProd.normalized();

        //Update the equation values
        sumAreaXNorm[idxA] += (tNormal * tArea);
        sumAreas[idxA] += tArea;
        sumAreaXNorm[idxB] += (tNormal * tArea);
        sumAreas[idxB] += tArea;
        sumAreaXNorm[idxC] += (tNormal * tArea);
        sumAreas[idxC] += tArea;
    }

    for (int i = 0; i < numVertex; i++) {
        Vector3R newNormal = sumAreaXNorm[i] / sumAreas[i];
        simNormals.segment<3>(3 * i) = newNormal;
        renderNormals.segment<3>(3 * i) = newNormal;
    }
}

void Object::localToWorld() {
    Eigen::Translation3f translation(0.0f, 0.0f, -5.0f);
    Eigen::Affine3f transform = Eigen::Affine3f::Identity() * translation;
    Eigen::Matrix4f modelMat = transform.matrix();

    for (int v = 0; v < positions.size(); v+=3) {
        Vector3R pos = positions.segment<3>(v);
        Eigen::Matrix<float, 4, 1> vert(pos.x(), pos.y(), pos.z(), 1.0);
        vert = modelMat * vert;
        positions.segment<3>(v) = vert.segment<3>(0);
    }
}


