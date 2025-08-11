#include "matching_engine.hpp"
#include "order_book.hpp"
#include "random_bot.hpp"

#include <iostream>
#include <thread>

int main()
{
    const int numBots = 8;
    const int ticks = 200;
    const int actionsPerTick = 2;
    auto matching_engine = std::make_shared<MatchingEngine>();
    // spawn bots
    std::vector<RandomBot> bots;
    for (int i = 0; i < numBots; ++i)
    {
        bots.emplace_back(matching_engine, i);
    }

    for (int t = 0; t < ticks; ++t) {
        for (auto &bot : bots) {
            for (int a = 0; a < actionsPerTick; ++a) {
                bot.run();
            }
        }

        // optional: print snapshots occasionally
        if (t % 20 == 0) {
            std::cout << "=== Tick " << t << " ===\n";
            matching_engine->print_order_book();
            std::cout << "Trades so far: " << matching_engine->get_trades().size() << "\n\n";
        }

        // light throttle so output is readable
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

        // final book and trades
    std::cout << "=== Final books ===\n";
    matching_engine->print_order_book();

    std::cout << "\nTrades executed: " << matching_engine->get_trades().size() << "\n";
    for (const auto &tr : matching_engine->get_trades()) {
        std::cout << "T: buyId=" << tr.buy_order_id << " sellId=" << tr.sell_order_id
                  << " sym=" << tr.symbol << " price=" << tr.price << " qty=" << tr.quantity << "\n";
    }
    return 0;
}
