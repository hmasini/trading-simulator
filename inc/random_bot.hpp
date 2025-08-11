#pragma once

#include "matching_engine.hpp"
#include <cstdint>
#include <random>
#include <vector>

using bot_id_t = uint64_t;

class RandomBot
{
public:
    RandomBot(std::shared_ptr<MatchingEngine> engine, bot_id_t id);
    // Runs the bot performing random actions based on probabilities
    void run();
private:
    bool coin(double probability);
    std::string get_random_symbol();
    double get_random_price_for_symbol(const std::string& symbol);
    uint get_random_quantity();
    bot_id_t m_id; 
    std::shared_ptr<MatchingEngine> m_matching_engine;
    // rng state (per-bot)
    std::mt19937 rng;

    // Track orders created by this bot
    std::vector<order_id_t> m_orders; 

    // Probabilities for different actions
    double m_new_order_probability = 0.8;
    double m_cancel_probability = 0.1;
    double m_amend_probability = 0.1;
};