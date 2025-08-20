#pragma once

#include <chrono>
#include <iomanip>
#include <string>

enum class Side 
{
    BUY,
    SELL
};

inline std::string to_string(Side side) 
{
    return side == Side::BUY ? "BUY" : "SELL";
}

using order_id_t = uint64_t;

struct Order
{
    order_id_t order_id;
    std::string symbol;
    Side side;
    double price;
    mutable uint quantity;
    std::chrono::time_point<std::chrono::system_clock> timestamp;

    Order(std::string symbol, Side side, double price, uint quantity)
        : symbol(symbol), side(side), price(price), quantity(quantity), timestamp(std::chrono::system_clock::now()) {}
};

inline std::string fmt_time(const std::chrono::system_clock::time_point& tp) {
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

inline std::string to_string(const Order& order) 
{
    return order.symbol + "," + to_string(order.side) + ", " + std::to_string(order.price) + ", " + std::to_string(order.quantity) + ", " + fmt_time(order.timestamp);
}

struct Trade
{
    order_id_t buy_order_id;
    order_id_t sell_order_id;
    std::string symbol;
    double price;
    uint quantity;
    std::chrono::time_point<std::chrono::system_clock> timestamp;

    Trade(int buy_order_id, int sell_order_id, std::string symbol, double price, uint quantity, std::chrono::time_point<std::chrono::system_clock> timestamp)
        : buy_order_id(buy_order_id), sell_order_id(sell_order_id), symbol(symbol), price(price), quantity(quantity), timestamp(timestamp) {}
};

inline std::string to_string(const Trade& trade) 
{
    return std::to_string(trade.buy_order_id) + "," + std::to_string(trade.sell_order_id) + "," + trade.symbol + "," + std::to_string(trade.price) + "," + std::to_string(trade.quantity) + "," + fmt_time(trade.timestamp);
}

enum class EventType {
    ADD,
    AMEND,
    CANCEL,
    TRADE
};

inline std::string to_string(EventType event) 
{
    switch (event) {
        case EventType::ADD: return "ADD";
        case EventType::AMEND: return "AMEND";
        case EventType::CANCEL: return "CANCEL";
        case EventType::TRADE: return "TRADE";
        default: return "UNKNOWN";
    }
}

