#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

namespace {
struct Sharing {
    explicit Sharing(int initialCoins)
        : coins(initialCoins), reserveLastCoin(initialCoins % 2 != 0) {}

    int coins;
    const bool reserveLastCoin;
    int bobCoins = 0;
    int tomCoins = 0;
    int deadCoins = 0;
    std::mutex mutex;
};

void coinSharing(const std::string& name, int& thiefCoins, int& companionCoins, Sharing& sharing) {
    while (true) {
        {
            std::lock_guard lock(sharing.mutex);
            if (sharing.coins == 0 || (sharing.reserveLastCoin && sharing.coins == 1)) {
                return;
            }

            if (thiefCoins <= companionCoins) {
                --sharing.coins;
                ++thiefCoins;
                std::cout << name << ": " << thiefCoins << " " << companionCoins
                          << ", в мешке: " << sharing.coins << '\n';
                continue;
            }
        }
        std::this_thread::yield();
    }
}

void shareCoins(int initialCoins) {
    Sharing sharing(initialCoins);
    std::cout << "\nНачальное количество монет: " << initialCoins << '\n';

    std::thread bob(coinSharing, "Боб", std::ref(sharing.bobCoins),
                    std::ref(sharing.tomCoins), std::ref(sharing));
    std::thread tom(coinSharing, "Том", std::ref(sharing.tomCoins),
                    std::ref(sharing.bobCoins), std::ref(sharing));
    bob.join();
    tom.join();

    sharing.deadCoins = sharing.coins;
    sharing.coins = 0;
    const int total = sharing.bobCoins + sharing.tomCoins + sharing.deadCoins;
    std::cout << "Итог: Боб = " << sharing.bobCoins
              << ", Том = " << sharing.tomCoins
              << ", покойник = " << sharing.deadCoins
              << ", сумма = " << total << '\n';
}
}

int main() {
    std::cout << "Пункт 1.\n";
    for (const int coins : {0, 1, 2, 5, 100, 101}) {
        shareCoins(coins);
    }
    return 0;
}
