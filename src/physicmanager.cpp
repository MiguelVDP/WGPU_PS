#include <physicmanager.h>

void PhysicManager::Initialize() {
    numDoFs = 0;

    for (auto it = simObjs.begin(); it != simObjs.end(); ++it) {
        numDoFs += it->getNumDoFs();
    }
}
