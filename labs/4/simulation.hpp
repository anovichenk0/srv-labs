#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string_view>
#include <thread>

namespace fatties {
struct Scenario {
    std::string_view title;
    int gluttony;
    int efficiencyFactor;
    std::chrono::milliseconds eatingPause;
};

enum class Phase { cook, eat, stopped };
enum class Outcome { none, fired, noSalary, resigned };

template<typename Mutex>
struct State {
    Mutex mutex;
    Phase phase = Phase::cook;
    Outcome outcome = Outcome::none;
    std::array<int, 3> dishes{3000, 3000, 3000};
    std::array<int, 3> eaten{};
    std::array<bool, 3> active{true, true, true};
    std::array<bool, 3> finishedEating{};
};

template<typename Mutex>
void fatty(std::size_t index, const Scenario& scenario, State<Mutex>& state) {
    while (true) {
        {
            std::unique_lock<Mutex> lock(state.mutex);
            while (state.phase != Phase::eat || state.finishedEating[index]) {
                if (state.phase == Phase::stopped) {
                    return;
                }
                lock.unlock();
                std::this_thread::yield();
                lock.lock();
            }

            if (state.active[index]) {
                const int portion = std::min(scenario.gluttony, state.dishes[index]);
                state.dishes[index] -= portion;
                state.eaten[index] += portion;
                if (state.eaten[index] > 10'000) {
                    state.active[index] = false;
                }
            }

            state.finishedEating[index] = true;
            if (std::all_of(state.finishedEating.begin(), state.finishedEating.end(),
                            [](bool finished) { return finished; })) {
                state.phase = Phase::cook;
            }
        }
        if (scenario.eatingPause.count() > 0) {
            std::this_thread::sleep_for(scenario.eatingPause);
        }
    }
}

template<typename Mutex>
void cook(const Scenario& scenario, State<Mutex>& state,
          std::chrono::steady_clock::time_point start) {
    while (true) {
        std::unique_lock<Mutex> lock(state.mutex);
        while (state.phase != Phase::cook) {
            lock.unlock();
            std::this_thread::yield();
            lock.lock();
        }

        if (std::none_of(state.active.begin(), state.active.end(), [](bool active) { return active; })) {
            state.outcome = Outcome::noSalary;
        } else if (std::any_of(state.dishes.begin(), state.dishes.end(),
                               [](int dish) { return dish == 0; })) {
            state.outcome = Outcome::fired;
        } else if (std::chrono::steady_clock::now() - start >= std::chrono::seconds(5)) {
            state.outcome = Outcome::resigned;
        } else {
            for (int& dish : state.dishes) {
                dish += scenario.efficiencyFactor;
            }
            state.finishedEating.fill(false);
            state.phase = Phase::eat;
            continue;
        }

        state.phase = Phase::stopped;
        return;
    }
}

inline std::string_view outcomeText(Outcome outcome) {
    switch (outcome) {
        case Outcome::fired: return "Кука уволили: закончились наггетсы хотя бы на одной тарелке";
        case Outcome::noSalary: return "Кук не получил зарплату: все три толстяка самоуничтожились";
        case Outcome::resigned: return "Кук уволился сам: прошло 5 дней";
        case Outcome::none: return "Работа завершилась без исхода";
    }
    return "Неизвестный исход";
}

template<typename Mutex>
void runScenario(const Scenario& scenario) {
    State<Mutex> state;
    const auto start = std::chrono::steady_clock::now();
    std::array<std::thread, 3> fatties{
        std::thread(fatty<Mutex>, 0, std::cref(scenario), std::ref(state)),
        std::thread(fatty<Mutex>, 1, std::cref(scenario), std::ref(state)),
        std::thread(fatty<Mutex>, 2, std::cref(scenario), std::ref(state)),
    };
    std::thread cookThread(cook<Mutex>, std::cref(scenario), std::ref(state), start);

    cookThread.join();
    for (auto& thread : fatties) {
        thread.join();
    }

    const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start);
    std::cout << '\n' << scenario.title << "\ngluttony = " << scenario.gluttony
              << ", efficiency_factor = " << scenario.efficiencyFactor << '\n'
              << outcomeText(state.outcome) << '\n'
              << "Съедено: " << state.eaten[0] << ", " << state.eaten[1] << ", "
              << state.eaten[2] << "; осталось: " << state.dishes[0] << ", "
              << state.dishes[1] << ", " << state.dishes[2] << '\n'
              << "Время: " << elapsed.count() << " с\n";
}

template<typename Mutex>
int runAllScenarios() {
    runScenario<Mutex>({"Исход 1", 3000, 1, std::chrono::milliseconds{0}});
    runScenario<Mutex>({"Исход 2", 100, 100, std::chrono::milliseconds{0}});
    runScenario<Mutex>({"Исход 3", 1, 1, std::chrono::milliseconds{1}});
    return 0;
}
}
