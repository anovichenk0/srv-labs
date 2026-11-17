#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <condition_variable>
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
    int selfDestructionLimit;
    std::chrono::milliseconds eatingPause;
};

enum class Phase { cook, eat, stopped };
enum class Outcome { none, fired, noSalary, resigned };

struct FattyState {
    int dish = 3000;
    int eaten = 0;
    bool active = true;
    bool finishedEating = false;
};

template<typename Mutex>
struct State {
    Mutex mutex;
    std::condition_variable_any cv;
    Phase phase = Phase::cook;
    Outcome outcome = Outcome::none;
    std::array<FattyState, 3> fatties;

    bool allFattiesInactive() const {
        return std::none_of(fatties.begin(), fatties.end(),
                            [](const FattyState& fatty) { return fatty.active; });
    }

    bool hasEmptyDish() const {
        return std::any_of(fatties.begin(), fatties.end(),
                           [](const FattyState& fatty) { return fatty.dish == 0; });
    }

    bool allFinishedEating() const {
        return std::all_of(fatties.begin(), fatties.end(),
                           [](const FattyState& fatty) { return fatty.finishedEating; });
    }

    bool fiveDaysPassed(std::chrono::steady_clock::time_point start) const {
        return std::chrono::steady_clock::now() - start >= std::chrono::seconds(5);
    }
};

template<typename Mutex>
void fatty(std::size_t index, const Scenario& scenario, State<Mutex>& state) {
    while (true) {
        {
            std::unique_lock<Mutex> lock(state.mutex);
            auto& self = state.fatties[index];

            // Ждем завершения либо что этот толстяк еще не ел
            state.cv.wait(lock, [&] {
                return state.phase == Phase::stopped
                    || (state.phase == Phase::eat && !self.finishedEating);
            });

            // Кук завершил сценарий и поток толстяка тоже завершается
            if (state.phase == Phase::stopped) {
                return;
            }

            // Самоуничтожившийся толстяк больше не ест
            if (self.active) {
                const int portion = std::min(scenario.gluttony, self.dish);
                self.dish -= portion;
                self.eaten += portion;

                // Превышен заданный предел — толстяк самоуничтожается.
                if (self.eaten > scenario.selfDestructionLimit) {
                    self.active = false;
                }
            }

            self.finishedEating = true;

            // Последний поевший передаёт следующий раунд куку
            if (state.allFinishedEating()) {
                state.phase = Phase::cook;
            }
            lock.unlock();
            state.cv.notify_all();
        }

        // В третьем сценарии толстяк спит после еды не удерживая mutex
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
        // Ждем фазу готовки
        state.cv.wait(lock, [&] { return state.phase == Phase::cook; });

        auto finish = [&](Outcome result) {
            state.outcome = result;
            state.phase = Phase::stopped;
            lock.unlock();
            state.cv.notify_all();
        };

        // Все толстяки самоуничтожились и Кук остаётся без зарплаты
        if (state.allFattiesInactive()) {
            finish(Outcome::noSalary);
            return;
        }

        // Хотя бы одна тарелка пуста и Кука увольняют
        if (state.hasEmptyDish()) {
            finish(Outcome::fired);
            return;
        }

        // Прошло пять условных дней и Кук увольняется сам
        if (state.fiveDaysPassed(start)) {
            finish(Outcome::resigned);
            return;
        }

        for (auto& fatty : state.fatties) {
            fatty.dish += scenario.efficiencyFactor;
            fatty.finishedEating = false;
        }
        state.phase = Phase::eat;
        lock.unlock();
        state.cv.notify_all();
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
              << "Съедено: " << state.fatties[0].eaten << ", " << state.fatties[1].eaten << ", "
              << state.fatties[2].eaten << "; осталось: " << state.fatties[0].dish << ", "
              << state.fatties[1].dish << ", " << state.fatties[2].dish << '\n'
              << "Время: " << elapsed.count() << " с\n";
}

template<typename Mutex>
int runAllScenarios() {
    runScenario<Mutex>({"Пункт 1.", 3000, 1, 10'000, std::chrono::milliseconds{0}});
    runScenario<Mutex>({"Пункт 2.", 100, 100, 10'000, std::chrono::milliseconds{0}});
    runScenario<Mutex>({"Пункт 3.", 1, 1, 10'000, std::chrono::milliseconds{1}});
    return 0;
}
}
