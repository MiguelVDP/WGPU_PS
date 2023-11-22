#ifndef OBJECT_H
#define OBJECT_H

#include <Eigen/Dense>

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using Vector3R = Eigen::Matrix<float, 3, 1>;
using Vectori = Eigen::Matrix<int, Eigen::Dynamic, 1>;

class Object{
public:
    //general
    VectorXR positions;
    Vectori triangles; //Vector of triangle indices.

    //Simulation
    VectorXR velocities;
    VectorXR accelerations;
    VectorXR masses;
    VectorXR simNormals;

    //Render
    VectorXR renderNormals;
    Vectori faces;
};

#endif