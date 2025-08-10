#include "matching_engine.hpp"
#include "order_book.hpp"

#include <iostream>

CentralLimitOrderBook& MatchingEngine::get_order_book_by_symbol(const std::string& symbol)
{
    return m_order_book_per_symbol[symbol];
}

void MatchingEngine::add_order(Order incoming_order) 
{
    // Generate a unique order ID
    incoming_order.order_id = m_next_order_id++;
    add_order_with_id(incoming_order, incoming_order.order_id);
}

bool MatchingEngine::cancel_order(order_id_t order_id)
{
    auto it = m_order_index.find(order_id);
    if (it == m_order_index.end()) { return false ;}

    auto set_it = it->second;
    auto& order_book = get_order_book_by_symbol(set_it->symbol);
    if (set_it->side == Side::BUY)
    {
        order_book.bids.erase(set_it);
    }
    else
    {
        order_book.asks.erase(set_it);
    }
    m_order_index.erase(it);

    return true;
}

bool MatchingEngine::amend_order(order_id_t order_id, double new_price, uint new_quantity)
{
    auto it = m_order_index.find(order_id);
    if (it == m_order_index.end()) { return false; }

    auto order_it = it->second;
    order_it->quantity = new_quantity;
    if (new_price != order_it->price)
    {
        Order amended_order = *order_it;        
        auto& order_book = get_order_book_by_symbol(order_it->symbol);
        if (order_it->side == Side::BUY)
        {
            order_book.bids.erase(it->second);
        }
        else
        {
            order_book.asks.erase(it->second);
        }
        amended_order.price = new_price;
        add_order_with_id(amended_order, amended_order.order_id);
    }
    return true;
}

// PRIVATE METHODS

void MatchingEngine::add_order_with_id(Order& order, order_id_t order_id)
{
    order.order_id = order_id;
    CentralLimitOrderBook& order_book = get_order_book_by_symbol(order.symbol);
    Order remaining_order = order;
    if (order.side == Side::BUY)
    {
        match_order(buyMatchesSell, remaining_order, order_book.asks);

        if (remaining_order.quantity > 0)
        {
            order_book.bids.insert(remaining_order);
            m_order_index[order.order_id] = order_book.bids.find(remaining_order);
        }
    }
    else
    {
        match_order(sellMatchesBuy, remaining_order, order_book.bids);

        if (remaining_order.quantity > 0)
        {
            order_book.asks.insert(remaining_order);
            m_order_index[order.order_id] = order_book.asks.find(remaining_order);
        }
    }
}

template<typename OppositeSide, typename PriceCondition>
void MatchingEngine::match_order(const PriceCondition& price_condition, Order& incoming_order, OppositeSide& opposite_side)
{
    auto it = opposite_side.begin();

    while (it != opposite_side.end() && price_condition(incoming_order, *it) && incoming_order.quantity > 0)
    {
        int match_quantity = std::min(incoming_order.quantity, it->quantity);

        m_trades.emplace_back(
            incoming_order.order_id, it->order_id, incoming_order.symbol, it->price, match_quantity, std::chrono::steady_clock::now());

        incoming_order.quantity -= match_quantity;

        if (it->quantity > match_quantity)
        {
            it->quantity -= match_quantity;
        }
        else
        {
            it = opposite_side.erase(it);
            m_order_index.erase(it->order_id);
        }
    }
}

