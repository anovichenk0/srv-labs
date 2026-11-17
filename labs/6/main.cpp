#include <chrono>
#include <coroutine>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

namespace {
struct NumberCoroutine {
    struct promise_type {
        NumberCoroutine get_return_object() {
            return NumberCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    explicit NumberCoroutine(std::coroutine_handle<promise_type> handle) : handle(handle) {}
    ~NumberCoroutine() { if (handle) handle.destroy(); }

    NumberCoroutine(const NumberCoroutine&) = delete;
    NumberCoroutine& operator=(const NumberCoroutine&) = delete;
};

NumberCoroutine printNumber(int value) {
    std::cout << "Пункт 1. Число из корутины: " << value << "\n\n";
    co_return;
}

struct ProgressTask {
    struct promise_type {
        int currentValue = 0;

        ProgressTask get_return_object() {
            return ProgressTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
        std::suspend_always yield_value(int value) noexcept {
            currentValue = value;
            return {};
        }
    };

    std::coroutine_handle<promise_type> handle;

    explicit ProgressTask(std::coroutine_handle<promise_type> handle) : handle(handle) {}
    ~ProgressTask() { if (handle) handle.destroy(); }
    ProgressTask(const ProgressTask&) = delete;
    ProgressTask& operator=(const ProgressTask&) = delete;

    void resume() { handle.resume(); }
    bool done() const { return handle.done(); }
    int value() const { return handle.promise().currentValue; }
};

ProgressTask longComputation(int steps) {
    for (int step = 1; step <= steps; ++step) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        co_yield step;
    }
}

void printProgress(int current, int total) {
    constexpr std::size_t width = 60;
    const std::string name = "ANDREY";
    const double percent = 100.0 * current / total;
    const std::size_t filled = width * current / total;

    std::string bar;
    bar.reserve(width);
    for (std::size_t position = 0; position < filled; ++position) {
        bar += name[position % name.size()];
    }
    bar.append(width - filled, ' ');

    std::cout << '\r' << bar << ' ' << std::fixed << std::setprecision(1)
              << percent << '%' << std::flush;
}
}

int main() {
    auto number = printNumber(42);
    number.handle.resume();

    constexpr int totalSteps = 100;
    auto progress = longComputation(totalSteps);
    while (!progress.done()) {
        progress.resume();
        if (!progress.done()) {
            printProgress(progress.value(), totalSteps);
        }
    }
    std::cout << "\nГотово\n";
    return 0;
}
