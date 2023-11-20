#include <physicmanager.h>

using VectorXR = Eigen::Matrix<float, Eigen::Dynamic, 1>;
using MatrixXR = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3R = Eigen::Matrix<float, 3, 1>;

void PhysicManager::initialize() {
    numDoFs = 0;

    for (auto & simObj : simObjs) {
        numDoFs += simObj.getNumDoFs();
    }
}
