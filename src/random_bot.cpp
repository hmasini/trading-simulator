#include "random_bot.hpp"
#include "matching_engine.hpp"

#include <random>
#include <stop_token>
#include <sys/types.h>


RandomBot::RandomBot(std::shared_ptr<MatchingEngine> engine, uint64_t id)
    : m_matching_engine(engine), m_id(id)
{
    std::random_device rd;
    rng.seed(rd() + m_id);
}

void RandomBot::start_thread()
{
    if (m_thread.joinable())
        return;

    m_thread = std::jthread([this](std::stop_token st) {
        while (!st.stop_requested()) {
            step();
        }
    });
}

// PRIVATE METHODS
void RandomBot::step()
{
    if (!m_orders.empty() && random_choice(CANCEL_PROBABILITY))
    {
        cancel_random_order();
        return;
    }
    
    if (!m_orders.empty() && random_choice(AMEND_PROBABILITY))
    {
        amend_random_order();
        return;
    }
    
    if (random_choice(NEW_PROBABILITY))
    {
        place_random_order();
    }
}

void RandomBot::cancel_random_order()
{
    std::uniform_int_distribution<size_t> di(0, m_orders.size() - 1);
    size_t idx = di(rng);
    order_id_t id_to_cancel = m_orders[idx];
    if (m_matching_engine->cancel_order(id_to_cancel))
    {
        m_orders.erase(m_orders.begin() + idx);
    }
}

void RandomBot::amend_random_order()
{
    std::uniform_int_distribution<size_t> di(0, m_orders.size()-1);
    size_t idx = di(rng);
    uint64_t id_to_amend = m_orders[idx];

    // simple amend: new quantity and slightly nudged price
    std::string symbol = pick_symbol();
    double new_price = generate_price_for_symbol(symbol);
    uint64_t new_quantity = generate_quantity();

    m_matching_engine->amend_order(id_to_amend, new_price, new_quantity);
}

void RandomBot::place_random_order()
{
    // create a new order
    std::string symbol = pick_symbol();
    double price = generate_price_for_symbol(symbol);
    uint64_t quantity = generate_quantity();

    Order new_order(symbol, random_choice(0.5) ? Side::BUY : Side::SELL, price, quantity);
    auto order_id = m_matching_engine->add_order(new_order);
    m_orders.push_back(order_id);
}

bool RandomBot::random_choice(double p)
{
    std::uniform_real_distribution<double> di(0.0, 1.0);
    return di(rng) < p;
}

std::string RandomBot::pick_symbol()
{
    std::uniform_int_distribution<size_t> di(0, ALLOWED_SYMBOLS.size() - 1);
    return ALLOWED_SYMBOLS[di(rng)];
}

uint RandomBot::generate_quantity()
{
    std::uniform_int_distribution<int> di(MIN_QUANTITY, MAX_QUANTITY);
    return di(rng);
}

double RandomBot::generate_price_for_symbol(const std::string& symbol)
{
    auto it = SYMBOLS_START_PRICES.find(symbol);
    if (it == SYMBOLS_START_PRICES.end()) return 100.0;

    auto start_price = it->second;
    std::normal_distribution<double> di(start_price, start_price * 0.05);
    return di(rng);
}