#pragma once

#include "order.hpp"
#include "order_book.hpp"
#include "logger.hpp"

#include <atomic>
#include <mutex>
#include <unordered_map>

class MatchingEngine
{
public:
    MatchingEngine(std::shared_ptr<Logger> logger);
    ~MatchingEngine() = default;
    // Adds an order to the matching engine and processes it against the order book
    order_id_t add_order(Order order);
    // Cancels an order by its ID
    // Returns true if the order was successfully canceled, false otherwise
    bool cancel_order(order_id_t order_id);
    // Amend an existing order, matching it against the order book
    // Returns true if the order was successfully amended, false otherwise
    bool amend_order(order_id_t order_id, double new_price, uint new_quantity);
    // Retrieves the order book for a specific symbol
    CentralLimitOrderBook& get_order_book_by_symbol(const std::string& symbol);
    // Retrieves the order books for all symbols
    const std::unordered_map<std::string, CentralLimitOrderBook>& get_order_book_for_all_symbols() const;
    // Retrieves the list of completed trades
    const std::vector<Trade>& get_trades() const { return m_trades; }
    // Aggregated top-of-book for a given symbol (copy under lock)
    std::vector<PriceLevel> get_top_bids(const std::string& symbol, size_t depth);
    std::vector<PriceLevel> get_top_asks(const std::string& symbol, size_t depth);
    // Recent trades snapshot (copy under lock, optionally filtered by symbol)
    std::vector<Trade> get_recent_trades(size_t count);
    std::vector<Trade> get_recent_trades_for_symbol(const std::string& symbol, size_t count);
private:
    // Match orders against the opposite side of the book
    template<typename OppositeSide, typename PriceCondition>
    void match_order(const PriceCondition& price_condition,Order& order, OppositeSide& opposite_side);
    // Adds an order while holding the mutex lock
    void locked_add_order(const Order& order);
    // Cancels an order while holding the mutex lock
    bool locked_cancel_order(order_id_t order_id);
    // Amends an order while holding the mutex lock
    bool locked_amend_order(order_id_t order_id, double new_price, uint new_quantity);
    // Map of order books by symbol
    std::unordered_map<std::string, CentralLimitOrderBook> m_order_book_per_symbol;
    // Track completed trades
    std::vector<Trade> m_trades;
    // Index map for order lookups
    std::unordered_map<order_id_t, std::multiset<Order>::iterator> m_order_index;
    // For generating unique order IDs
    std::atomic<order_id_t> m_next_order_id{1};
    // Logger instance for logging orders and trades
    std::shared_ptr<Logger> m_logger;

    std::mutex m_mutex;
};