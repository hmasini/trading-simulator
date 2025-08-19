#include "matching_engine.hpp"
#include "random_bot.hpp"
#include "cli.hpp"

#include <memory>
#include <ncurses.h>
#include <chrono>
#include <thread>
#include <atomic>

int main() {

    auto matching_engine = std::make_shared<MatchingEngine>();

    const size_t depth = 50;
    const size_t lastTrades = 12;

    // --- start bots ---
    std::vector<std::unique_ptr<RandomBot>> bots;
    const int numBots = 8;
    
    for (int i = 0; i < numBots; ++i)
        bots.emplace_back(std::make_unique<RandomBot>(matching_engine, i + 1));
    for (auto& b : bots) b->start_thread();

    init_cli();

    std::atomic<bool> running{true};

    // --- UI CLI loop ---
    while (running)
    {
        erase();
        auto order_book_for_all_symbols = matching_engine->get_order_book_for_all_symbols();

        render_market_dashboard(matching_engine, depth, lastTrades, running);
        std::this_thread::sleep_for(std::chrono::milliseconds(80)); // ~12.5 FPS
    }

    // --- teardown ---
    shutdown_cli();
    for (auto& bot : bots) bot->stop_thread();

    return 0;
}