#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

namespace {
std::mutex m;

void func(const std::string& name, bool useMutex) {
    long double i = 0;
    const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(1);

   while (std::chrono::steady_clock::now() < end) {
        i += 1e-9L;
    }

    if (useMutex) {
        m.lock();
        std::cout << name << ": " << std::fixed << std::setprecision(12) << i << std::endl;
        m.unlock();

        return;
    }

    std::cout << name << ": " << std::fixed << std::setprecision(12) << i << std::endl;
}

void runThreeThreads(bool useMutex) {
    std::cout << (useMutex ? "Пункт 1.\n" : "Пункт 2.\n");
    std::thread thread1(func, "t1", useMutex);
    std::thread thread2(func, "t2", useMutex);
    std::thread thread3(func, "t3", useMutex);
    thread1.join();
    thread2.join();
    thread3.join();
}
}

int main() {
    runThreeThreads(true);
    runThreeThreads(false);
    return 0;
}
