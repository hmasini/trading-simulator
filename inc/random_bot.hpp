#pragma once

#include "matching_engine.hpp"
#include <cstdint>
#include <random>
#include <thread>
#include <vector>

class RandomBot
{
public:
    RandomBot(std::shared_ptr<MatchingEngine> engine, uint64_t id);
    // Starts the bot's thread - performs random actions like placing, amending, or canceling orders
    void start_thread();
private:
    // Bot's action logic
    void step();
    // Places a new order with random parameters
    void place_random_order();
    // Cancels a random order from the bot's order list
    void cancel_random_order();
    // Amends a random order from the bot's order list
    void amend_random_order();
    // Generates a random choice based on the given probability
    bool random_choice(double probability);
    // Picks a random symbol from the allowed symbols based on uniform distribution
    std::string pick_symbol();
    // Returns a random price for the given symbol given a start price and a random variation
    double generate_price_for_symbol(const std::string& symbol);
    // Generates a random quantity based on a uniform distribution
    uint generate_quantity();

    uint64_t m_id; 
    std::shared_ptr<MatchingEngine> m_matching_engine;
    // rng state (per-bot)
    std::mt19937 rng;
    // Track orders created by this bot
    std::vector<order_id_t> m_orders; 

    std::jthread m_thread;

    // Constants
    static constexpr uint MIN_QUANTITY = 1;
    static constexpr uint MAX_QUANTITY = 20;
    // Probabilities for different actions
    static constexpr double NEW_PROBABILITY = 0.8;
    static constexpr double CANCEL_PROBABILITY = 0.1;
    static constexpr double AMEND_PROBABILITY = 0.1;
    
    static inline const std::unordered_map<std::string, double> SYMBOLS_START_PRICES = {
        {"AAPL", 226.5},
        {"GOOG", 201.5},
        {"NVDA", 183.0},
        {"NFLX", 1218.2}
    };
    // Creates a vector of allowed symbols statically
    static inline const std::vector<std::string> ALLOWED_SYMBOLS = []{
        std::vector<std::string> allowed_symbols;
        std::transform(
            SYMBOLS_START_PRICES.begin(), SYMBOLS_START_PRICES.end(),
            std::back_inserter(allowed_symbols),
            [](const auto& pair) { return pair.first; }
        );
        return allowed_symbols;
    }();
};