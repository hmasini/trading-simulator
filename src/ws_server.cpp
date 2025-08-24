// WebSocket server implementation

#include "matching_engine.hpp"
#include "order_book.hpp"
#include "order_types.hpp"
#include "random_bot.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <csignal>

using json = nlohmann::json;
using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

std::atomic<bool> running{true};

void signal_handler(int)
{
    running = false;
}

// --- parsers ---
void to_json(json& j, const PriceLevel& level)
{
    j = json{
        {"price", level.price},
        {"quantity", level.quantity}
    };
}

void to_json(json& j, const OrderBookSnapshot& snapshot)
{
    j = json{
        {"symbol", snapshot.symbol},
        {"bids", snapshot.bids},
        {"asks", snapshot.asks}
    };
}

void to_json(json& j, const std::vector<OrderBookSnapshot>& snapshots)
{
    j = json::array();
    for (const auto& snapshot : snapshots)
    {
        json jsnap;
        to_json(jsnap, snapshot);
        j.push_back(jsnap);
    }
}

void to_json(json& j, const Trade& trade)
{
    j = json{
        {"symbol", trade.symbol},
        {"price", trade.price},
        {"quantity", trade.quantity},
        {"timestamp", fmt_time(trade.timestamp).c_str()}
    };
}

void to_json(json& j, const std::vector<Trade>& trades)
{
    j = json::array();
    for (const auto& trade : trades)
    {
        json jtrade;
        to_json(jtrade, trade);
        j.push_back(jtrade);
    }
}

int main() 
{
    std::signal(SIGINT, signal_handler);

    // --- Matching engine with no logger ---
    auto matching_engine = std::make_shared<MatchingEngine>();

    // --- start bots ---
    std::vector<std::unique_ptr<RandomBot>> bots;
    const int numBots = 8;
    
    for (int i = 0; i < numBots; ++i)
        bots.emplace_back(std::make_unique<RandomBot>(matching_engine, i + 1));
    for (auto& b : bots) b->start_thread();

    boost::asio::io_context ioc;
    tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 9001));
    std::cout << "WebSocket server listening on port 9001\n";
    while (running)
    {
        tcp::socket socket(ioc);
        acceptor.accept(socket);
        websocket::stream<tcp::socket> ws(std::move(socket));
        ws.accept();
        std::cout << "Client connected\n";
        try
        {
            while (running)
            {
                json j;
                to_json(j["order_books"], matching_engine->get_order_book_snapshot());
                to_json(j["recent_trades"], matching_engine->get_recent_trades());
                std::cout << j.dump() << "\n";
                ws.write(boost::asio::buffer(j.dump()));
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "Sent order book snapshot\n";
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Websocket error: " << e.what() << "\n";
            break;
        }
    }

    // --- teardown ---
    for (auto& bot : bots) bot->stop_thread();
    std::cout << "Server shutting down...\n";
    return 0;
}