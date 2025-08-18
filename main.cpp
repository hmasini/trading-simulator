#include "matching_engine.hpp"
#include "order_book.hpp"
#include "random_bot.hpp"

#include <cstddef>
#include <iostream>
#include <iomanip>
#include <thread>

constexpr int NUM_BOTS = 8;

// ANSI color codes
const std::string RESET   = "\033[0m";
const std::string RED     = "\033[31m";   // for asks
const std::string GREEN   = "\033[32m";   // for bids
const std::string YELLOW  = "\033[33m";   // optional: for spread

void displayOrderBook(const std::unordered_map<std::string, CentralLimitOrderBook>& get_order_book_for_all_symbols, size_t depth = 5, size_t lastTrades = 5) {
    for (const auto& [symbol, book] : get_order_book_for_all_symbols) {
        auto bids = book.bids;
        auto asks = book.asks;

        std::cout << "\n===== ORDER BOOK =====\n";
        std::cout << "-> " << symbol << " <-\n";
        std::cout << std::left << std::setw(20) << "BID (Qty@Price)"
                << " | "
                << std::setw(20) << "ASK (Qty@Price)" << "\n";
        std::cout << "----------------------+----------------------\n";

        // Create vectors from multisets (easier to index)
        std::vector<Order> bidVec(bids.begin(), bids.end());
        std::vector<Order> askVec(asks.begin(), asks.end());

        size_t maxSize = std::max(bidVec.size(), askVec.size());
        for (size_t i = 0; i < maxSize; i++) {
            std::ostringstream bidStr, askStr;

            if (i < bidVec.size())
                bidStr << bidVec[i].quantity << "@" << bidVec[i].price;
            if (i < askVec.size())
                askStr << askVec[i].quantity << "@" << askVec[i].price;

            // Color formatting
            std::string bidOutput = (i < bidVec.size())
                ? GREEN + bidStr.str() + RESET : "";
            std::string askOutput = (i < askVec.size())
                ? RED + askStr.str() + RESET : "";

            std::cout << std::left << std::setw(20) << bidStr.str()
                    << " | "
                    << std::setw(20) << askStr.str()
                    << "\n";
        }

        std::cout << "======================\n";
    }
}

int main()
{
    auto matching_engine = std::make_shared<MatchingEngine>();

    {
        std::vector<RandomBot> bots;
        bots.reserve(NUM_BOTS);

        for (int i = 0; i < NUM_BOTS; ++i)
            bots.emplace_back(matching_engine, i);
        for (auto& bot: bots)
            bot.start_thread();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    auto trades = matching_engine->get_trades();
    displayOrderBook(matching_engine->get_order_book_for_all_symbols());

    return 0;
}
