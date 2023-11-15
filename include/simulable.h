#pragma once

#include <physicmanager.h>
#include <Eigen/Dense>
#include "physicmanager.h"

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using MatrixXR = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;

class Simulable{
public:
    /// <summary>
    /// Initialize the simulable.
    /// </summary>
    virtual void Initialize(int i, PhysicManager manager) = 0;

    /// <summary>
    /// Returns the number of model DOF.
    /// </summary>
    virtual int getNumDoFs() = 0;

    /// <summary>
    /// Write position values into the position vector.
    /// </summary>
    virtual void getPosition(VectorXR position) = 0;

    /// <summary>
    /// Set position values from the position vector.
    /// </summary>
    virtual void setPosition(VectorXR position) = 0;

    /// <summary>
    /// Write velocity values into the velocity vector.
    /// </summary>
    virtual void getVelocity(VectorXR velocity) = 0;

    /// <summary>
    /// Set velocity values from the velocity vector.
    /// </summary>
    virtual void setVelocity(VectorXR velocity) = 0;

    /// <summary>
    /// Write force values into the force vector.
    /// </summary>
    virtual void getFore(VectorXR force) = 0;

    /// <summary>
    /// Write force jacobian values into the matrix.
    /// </summary>
    virtual void getForceJacobian(MatrixXR dFdx, MatrixXR dFdv) = 0;

    /// <summary>
    /// Write mass values into the mass matrix.
    /// </summary>
    virtual void GetMass(MatrixXR mass) = 0;

    /// <summary>
    /// Write inverse of mass values into the inverse mass matrix.
    /// </summary>
    virtual void GetMassInverse(MatrixXR massInv) = 0;
};