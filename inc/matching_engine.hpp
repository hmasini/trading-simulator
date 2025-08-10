#pragma once

#include "order.hpp"
#include "order_book.hpp"

class MatchingEngine
{
public:
    // Adds an order to the matching engine and processes it against the order book
    void add_order(Order order);
    // Cancels an order by its ID
    // Returns true if the order was successfully canceled, false otherwise
    bool cancel_order(order_id_t order_id);
    // Amend an existing order, matching it against the order book
    // Returns true if the order was successfully amended, false otherwise
    bool amend_order(order_id_t order_id, double new_price, uint new_quantity);
    // Retrieves the order book for a specific symbol
    CentralLimitOrderBook& get_order_book_by_symbol(const std::string& symbol);
    // Retrieves the list of completed trades
    const std::vector<Trade>& get_trades() const { return m_trades; }
private:
    // Match orders against the opposite side of the book
    template<typename OppositeSide, typename PriceCondition>
    void match_order(const PriceCondition& price_condition,Order& order, OppositeSide& opposite_side);
    // Add an order with a unique ID
    void add_order_with_id(Order& order, order_id_t order_id);
    // Map of order books by symbol
    std::unordered_map<std::string, CentralLimitOrderBook> m_order_book_per_symbol;
    // Track completed trades
    std::vector<Trade> m_trades;
    // Index map for order lookups
    std::unordered_map<order_id_t, std::multiset<Order>::iterator> m_order_index;
    // For generating unique order IDs
    order_id_t m_next_order_id = 1;
};