#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

namespace {
constexpr std::uint64_t iterations = 10'000'000;

std::uint64_t factorial10() {
    std::uint64_t result = 1;
    for (unsigned int value = 2; value <= 10; ++value) {
        result *= value;
    }
    return result;
}

std::uint64_t calculateFactorials() {
    volatile std::uint64_t result = 0;
    for (std::uint64_t iteration = 0; iteration < iterations; ++iteration) {
        result = factorial10();
    }
    return result;
}

void func(const std::string& name) {
    const auto result = calculateFactorials();
    std::cout << name << ": 10! = " << result << ", вычислено " << iterations << " раз\n";
}

}

int main() {
    std::cout << "Пункт 1.1.\n";
    const std::string s = "01234";
    // Исправленный вывод
    for (std::size_t i = s.size(); i-- > 0;) {
        std::cout << s[i] << '\n';
    }

    std::cout << "\nПункт 1.2.\n";
    const auto sequentialCpuStart = std::clock();
    const auto sequentialWallStart = std::chrono::steady_clock::now();

    func("t1 (последовательно)");
    func("t2 (последовательно)");

    const double sequentialCpu = static_cast<double>(std::clock() - sequentialCpuStart) / CLOCKS_PER_SEC;
    const double sequentialWall = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - sequentialWallStart).count();
    std::cout << std::fixed << std::setprecision(6)
              << "Два последовательных вызова: CPU = " << sequentialCpu
              << " с, реальное время = " << sequentialWall << " с\n";

    std::cout << "Пункт 1.3.\n";
    const auto parallelCpuStart = std::clock();
    const auto parallelWallStart = std::chrono::steady_clock::now();

    std::thread thread1(func, "t1 (параллельно)");
    std::thread thread2(func, "t2 (параллельно)");
    thread1.join();
    thread2.join();

    const double parallelCpu = static_cast<double>(std::clock() - parallelCpuStart) / CLOCKS_PER_SEC;
    const double parallelWall = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - parallelWallStart).count();
    std::cout << "Два параллельных потока: CPU = " << parallelCpu
              << " с, реальное время = " << parallelWall << " с\n";
    return 0;
}
