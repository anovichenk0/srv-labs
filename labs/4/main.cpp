#include "simulation.hpp"

#include <mutex>

int main() {
    return fatties::runAllScenarios<std::mutex>();
}
