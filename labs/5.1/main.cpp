#include "../4/simulation.hpp"

#include <semaphore>

class SemaphoreMutex {
public:
    void lock() {
        available.acquire();
    }

    void unlock() {
        available.release();
    }

private:
    std::binary_semaphore available{1};
};

int main() {
    return fatties::runAllScenarios<SemaphoreMutex>();
}
