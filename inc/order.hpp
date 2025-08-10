#pragma once

#include <chrono>
#include <string>

enum class Side 
{
    BUY,
    SELL
};

using order_id_t = uint64_t;

struct Order
{
    order_id_t order_id;
    std::string symbol;
    Side side;
    double price;
    mutable uint quantity;
    std::chrono::time_point<std::chrono::steady_clock> timestamp;

    Order(std::string symbol, Side side, double price, uint quantity)
        : symbol(symbol), side(side), price(price), quantity(quantity), timestamp(std::chrono::steady_clock::now()) {}
};

struct Trade
{
    order_id_t buy_order_id;
    order_id_t sell_order_id;
    std::string symbol;
    double price;
    uint quantity;
    std::chrono::time_point<std::chrono::steady_clock> timestamp;

    Trade(int buy_order_id, int sell_order_id, std::string symbol, double price, uint quantity, std::chrono::time_point<std::chrono::steady_clock> timestamp)
        : buy_order_id(buy_order_id), sell_order_id(sell_order_id), symbol(symbol), price(price), quantity(quantity), timestamp(timestamp) {}
};

