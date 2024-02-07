#ifndef OBJECT_H
#define OBJECT_H

#include <Eigen/Dense>
#include <iostream>

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using Vector3R = Eigen::Matrix<float, 3, 1>;
using Vector16i = Eigen::Matrix<uint16_t , Eigen::Dynamic, 1>;
using Vector32i = Eigen::Matrix<uint32_t , Eigen::Dynamic, 1>;

class Object{
public:
    //general
    VectorXR positions;
    Vector16i triangles; //Vector of triangle indices.

    //Simulation
    VectorXR velocities;
    VectorXR accelerations;
    VectorXR masses;
    VectorXR simNormals;

    //Render
    VectorXR renderNormals;
    Vector16i faces;

    void computeNormals();

    void localToWorld();
};

#endif