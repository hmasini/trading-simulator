#pragma once
#include <set>

#include "order.hpp"


// price-time priority for orders
struct BidComparator
{
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) {
            return a.price > b.price;
        }
        return a.timestamp < b.timestamp;
    }
};

struct AskComparator
{
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) {
            return a.price < b.price;
        }
        return a.timestamp < b.timestamp;
    }
};

// Matching pricing convinience functions
static constexpr auto buyMatchesSell =
    [](const Order& buy, const Order& sell) {
        return buy.price >= sell.price;
    };

static constexpr auto sellMatchesBuy =
    [](const Order& sell, const Order& buy) {
        return sell.price <= buy.price;
    };

struct CentralLimitOrderBook
{
    std::multiset<Order, BidComparator> bids;
    std::multiset<Order, AskComparator> asks;
};

struct PriceLevel {
    double price;
    uint64_t quantity;
};
