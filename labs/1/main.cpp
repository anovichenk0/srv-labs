#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
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

struct Measurement {
    double cpuSeconds;
    double wallSeconds;
};

template<typename Function>
Measurement measure(Function&& function) {
    const auto cpuStart = std::clock();
    const auto wallStart = std::chrono::steady_clock::now();
    std::invoke(std::forward<Function>(function));
    const auto wallEnd = std::chrono::steady_clock::now();
    const auto cpuEnd = std::clock();

    return {
        static_cast<double>(cpuEnd - cpuStart) / CLOCKS_PER_SEC,
        std::chrono::duration<double>(wallEnd - wallStart).count(),
    };
}

void printMeasurement(const std::string& title, const Measurement& measurement) {
    std::cout << std::fixed << std::setprecision(6)
              << title << ": CPU = " << measurement.cpuSeconds
              << " с, реальное время = " << measurement.wallSeconds << " с\n";
}
}

int main() {
    std::cout << "Пункт 1.1. Безопасный обратный обход строки:\n";
    const std::string s = "01234";
    for (std::size_t i = s.size(); i-- > 0;) {
        std::cout << s[i] << '\n';
    }
    std::cout << "В исходном цикле после i == 0 unsigned int переходит к UINT_MAX;\n"
                 "условие i >= 0 остаётся истинным, поэтому далее доступ к s[i] небезопасен.\n\n";

    std::cout << "Пункты 1.2–1.3. Вычисление 10! по " << iterations
              << " раз в каждом вызове\n";
    const auto sequential = measure([] {
        func("t1 (последовательно)");
        func("t2 (последовательно)");
    });
    printMeasurement("Два последовательных вызова", sequential);

    const auto parallel = measure([] {
        std::thread thread1(func, "t1 (параллельно)");
        std::thread thread2(func, "t2 (параллельно)");
        thread1.join();
        thread2.join();
    });
    printMeasurement("Два параллельных потока", parallel);
    return 0;
}
