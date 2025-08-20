#include "matching_engine.hpp"
#include "logger.hpp"
#include "order.hpp"
#include "order_book.hpp"

#include <atomic>
#include <mutex>
#include <map>

MatchingEngine::MatchingEngine(std::shared_ptr<Logger> logger)
    : m_logger(std::move(logger))
{
}

const std::unordered_map<std::string, CentralLimitOrderBook>& MatchingEngine::get_order_book_for_all_symbols() const
{
    return m_order_book_per_symbol;
}

order_id_t MatchingEngine::add_order(Order incoming_order) 
{
    // Generate a unique order ID
    incoming_order.order_id = m_next_order_id.fetch_add(1, std::memory_order_relaxed);

    std::lock_guard<std::mutex> lock(m_mutex);
    locked_add_order(incoming_order);

    m_logger->log_order(EventType::ADD, to_string(incoming_order));
    
    return incoming_order.order_id;
}

bool MatchingEngine::cancel_order(order_id_t order_id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return locked_cancel_order(order_id);
}

bool MatchingEngine::amend_order(order_id_t order_id, double new_price, uint new_quantity)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return locked_amend_order(order_id, new_price, new_quantity);
}

CentralLimitOrderBook& MatchingEngine::get_order_book_by_symbol(const std::string& symbol)
{
    return m_order_book_per_symbol[symbol];
}

std::vector<PriceLevel> MatchingEngine::get_top_bids(const std::string& symbol, size_t depth) 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PriceLevel> out;
    auto it = m_order_book_per_symbol.find(symbol);
    if (it == m_order_book_per_symbol.end()) return out;

    // aggregate quantities by price
    std::map<double, uint64_t, std::greater<>> agg;
    for (const auto& o : it->second.bids) agg[o.price] += o.quantity;

    out.reserve(std::min(depth, agg.size()));
    for (auto a = agg.begin(); a != agg.end() && out.size() < depth; ++a)
        out.push_back(PriceLevel{a->first, a->second});
    return out;
}

std::vector<PriceLevel> MatchingEngine::get_top_asks(const std::string& symbol, size_t depth) 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PriceLevel> out;
    auto it = m_order_book_per_symbol.find(symbol);
    if (it == m_order_book_per_symbol.end()) return out;

    std::map<double, uint64_t> agg;
    for (const auto& o : it->second.asks) agg[o.price] += o.quantity;

    out.reserve(std::min(depth, agg.size()));
    for (auto a = agg.begin(); a != agg.end() && out.size() < depth; ++a)
        out.push_back(PriceLevel{a->first, a->second});
    return out;
}

std::vector<Trade> MatchingEngine::get_recent_trades(size_t count)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_trades.size() <= count) return m_trades;
    return std::vector<Trade>(m_trades.end() - count, m_trades.end());
}

std::vector<Trade> MatchingEngine::get_recent_trades_for_symbol(const std::string& symbol, size_t count)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Trade> filtered;
    filtered.reserve(std::min(count, m_trades.size()));
    for (auto it = m_trades.rbegin(); it != m_trades.rend() && filtered.size() < count; ++it)
        if (it->symbol == symbol) filtered.push_back(*it);
    std::reverse(filtered.begin(), filtered.end());
    return filtered;
}

// PRIVATE METHODS
void MatchingEngine::locked_add_order(const Order& order)
{
    CentralLimitOrderBook& order_book = get_order_book_by_symbol(order.symbol);
    Order remaining_order = order;

    auto insert_order = [&](auto &order_book) {
        order_book.insert(remaining_order);
        m_order_index[remaining_order.order_id] = order_book.find(remaining_order);
    };
    
    if (order.side == Side::BUY)
    {
        match_order(buyMatchesSell, remaining_order, order_book.asks);

        if (remaining_order.quantity > 0)
            insert_order(order_book.bids);
    }
    else
    {
        match_order(sellMatchesBuy, remaining_order, order_book.bids);

        if (remaining_order.quantity > 0)
            insert_order(order_book.asks);
    }
}

bool MatchingEngine::locked_cancel_order(order_id_t order_id)
{
    auto it = m_order_index.find(order_id);
    if (it == m_order_index.end()) { return false ;}

    auto set_it = it->second;
    m_logger->log_order(EventType::CANCEL, to_string(*set_it));

    auto& order_book = get_order_book_by_symbol(set_it->symbol);
    m_order_index.erase(it);
    if (set_it->side == Side::BUY)
    {
        order_book.bids.erase(set_it);
    }
    else
    {
        order_book.asks.erase(set_it);
    }

    return true;
}

bool MatchingEngine::locked_amend_order(order_id_t order_id, double new_price, uint new_quantity)
{
    auto it = m_order_index.find(order_id);
    if (it == m_order_index.end()) { return false; }

    auto order_it = it->second;
    m_logger->log_order(EventType::AMEND, to_string(*it->second));

    order_it->quantity = new_quantity;

    if (new_price != order_it->price)
    {
        auto side = order_it->side;
        auto symbol = order_it->symbol;
        Order amended_order = *order_it;        
        auto& order_book = get_order_book_by_symbol(symbol);
        
        m_order_index.erase(it);

        if (side == Side::BUY)
            order_book.bids.erase(order_it);
        else
            order_book.asks.erase(order_it);

        amended_order.price = new_price;
        locked_add_order(amended_order);
    }

    return true;
}

template<typename OppositeSide, typename PriceCondition>
void MatchingEngine::match_order(const PriceCondition& price_condition, Order& incoming_order, OppositeSide& opposite_side)
{
    auto it = opposite_side.begin();

    while (it != opposite_side.end() && price_condition(incoming_order, *it) && incoming_order.quantity > 0)
    {
        int match_quantity = std::min(incoming_order.quantity, it->quantity);

        m_trades.emplace_back(
            incoming_order.order_id, it->order_id, incoming_order.symbol, it->price, match_quantity, std::chrono::system_clock::now());

        m_logger->log_order(EventType::TRADE, to_string(m_trades.back()));

        incoming_order.quantity -= match_quantity;

        if (it->quantity > match_quantity)
        {
            it->quantity -= match_quantity;
        }
        else
        {
            m_order_index.erase(it->order_id);
            it = opposite_side.erase(it);
        }
    }
}
