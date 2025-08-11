#include "random_bot.hpp"
#include "matching_engine.hpp"
#include <iostream>

constexpr size_t num_symbols = 4;
constexpr std::array<std::string, num_symbols> allowed_symbols{"AAPL", "GOOG", "NVDA", "NFLX"};
constexpr std::array<double, num_symbols> start_prices{226.5, 201.5, 183.0, 1218.2};

RandomBot::RandomBot(std::shared_ptr<MatchingEngine> engine, bot_id_t id)
    : m_matching_engine(engine), m_id(id)
{
    rng.seed(m_id); // Seed once here
}

bool RandomBot::coin(double p)
{
    std::uniform_real_distribution<double> d(0.0, 1.0);
    return d(rng) < p;
}

std::string RandomBot::get_random_symbol()
{
    std::uniform_int_distribution<size_t> di(0, allowed_symbols.size() - 1);
    auto sm =  allowed_symbols[di(rng)];
    return sm;
}

double RandomBot::get_random_price_for_symbol(const std::string& symbol)
{
    auto it = std::find(allowed_symbols.begin(), allowed_symbols.end(), symbol);
    if (it != allowed_symbols.end())
    {
        size_t index = std::distance(allowed_symbols.begin(), it);
        return start_prices[index];
    }
    return 100.0;
}

uint RandomBot::get_random_quantity()
{
    std::discrete_distribution<int> dd({60,30,8,2}); // bias small sizes
    int tier = dd(rng) + 1;
    return static_cast<uint64_t>(tier * 10); // 10,20,30,40...
}

void RandomBot::run()
{
    // decide action
    if (!m_orders.empty() && coin(m_cancel_probability))
    {
        // cancel a random order
        std::uniform_int_distribution<size_t> di(0, m_orders.size() - 1);
        size_t idx = di(rng);
        order_id_t idToCancel = m_orders[idx];
        if (m_matching_engine->cancel_order(idToCancel))
        {
            m_orders.erase(m_orders.begin() + idx);
        }
        return;
    }

    if (!m_orders.empty() && coin(m_amend_probability))
    {
        // amend a random one (either quantity or price or both)
        std::uniform_int_distribution<size_t> di(0, m_orders.size()-1);
        size_t idx = di(rng);
        uint64_t idToAmend = m_orders[idx];

        // simple amend: new quantity and slightly nudged price
        std::string symbol = get_random_symbol();
        double newPrice = get_random_price_for_symbol(symbol);
        uint64_t newQty = get_random_quantity();

        m_matching_engine->amend_order(idToAmend, newPrice, newQty);
        return;
    }

    if (coin(m_new_order_probability))
    {
        // create a new order
        std::string symbol = get_random_symbol();
        double price = get_random_price_for_symbol(symbol);
        uint64_t quantity = get_random_quantity();

        Order newOrder(symbol, coin(0.5) ? Side::BUY : Side::SELL, price, quantity);
        auto order_id = m_matching_engine->add_order(newOrder);
        m_orders.push_back(order_id);
    }
    
}