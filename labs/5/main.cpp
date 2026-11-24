#include "../4/simulation.hpp"

#include <atomic>
#include <thread>

class SpinMutex {
public:
    void lock() noexcept {
        while (true) {
            // Переменная expected используется всего 1 раз.
            // Просто функция выполняет запись и нельзя заинлайнить значение
            bool expected = false;
            if (locked.compare_exchange_weak(expected, true, std::memory_order_acquire)) {
                // Поток забирает владение и выходит из lock
                return;
            }
            while (locked.load(std::memory_order_relaxed)) {
                // Пока залочено продолжаем отдавать владение
                std::this_thread::yield();
            }
        }
    }

    void unlock() noexcept {
        locked.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> locked{false};
};

int main() {
    return fatties::runAllScenarios<SpinMutex>();
}
