#include "cli.hpp"

#include <iomanip>
#include <ncurses.h>
#include <sstream>

// --- helpers ---
static std::string fmt_px(double p) {
    std::ostringstream oss; oss.setf(std::ios::fixed); oss<<std::setprecision(2)<<p; return oss.str();
}
static std::string fmt_qty(uint64_t q) {
    return std::to_string(q);
}

static void render_symbol_header(const std::string& symbol, size_t depth, int cols, int asks_col)
{
    attron(A_BOLD);
    mvprintw(0, 0, "CLOB Viewer  |  Symbol: %s  |  Depth: %zu  |  q=quit  arrows=switch  +/- depth", symbol.c_str(), depth);
    attroff(A_BOLD);

    mvprintw(2, 2, "BIDS (Qty @ Price)");
    mvprintw(2, asks_col + 2, "ASKS (Qty @ Price)");
    mvhline(3, 1, '-', 50);

}

static void render_order_book_sides(const std::vector<PriceLevel>& bids, const std::vector<PriceLevel>& asks, size_t depth, int asks_col, size_t max_rows, int rows)
{
    // colors
    // 1: GREEN on BLACK for bids
    // 2: RED on BLACK   for asks
    // 3: YELLOW for spread/header highlights

    // render side-by-side
    for (size_t i = 0; i < max_rows && (int)i < rows - 8; ++i) 
    {
        // bids on left
        if (i < bids.size())
        {
            attron(COLOR_PAIR(1));
            mvprintw(4 + (int)i, 2, "%8s @ %10s",
                    fmt_qty(bids[i].quantity).c_str(),
                    fmt_px(bids[i].price).c_str());
            attroff(COLOR_PAIR(1));
        }
        else
        {
            mvprintw(4 + (int)i, 2, "%8s   %10s", "", "");
        }

        // asks on right
        if (i < asks.size())
        {
            attron(COLOR_PAIR(2));
            mvprintw(4 + (int)i, asks_col, "%8s @ %10s",
                    fmt_qty(asks[i].quantity).c_str(),
                    fmt_px(asks[i].price).c_str());
            attroff(COLOR_PAIR(2));
        }
        else
        {
            mvprintw(4 + (int)i, asks_col + 2, "%8s   %10s", "", "");
        }
    }
}

static void render_best_bid_ask_spread(const std::vector<PriceLevel>& bids, const std::vector<PriceLevel>& asks, size_t max_rows)
{
    if (!bids.empty() && !asks.empty())
    {
        double best_bid = bids.front().price;
        double best_ask = asks.front().price;
        double spread = best_ask - best_bid;
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(4 + (int)max_rows + 1, 2, "Best Bid %s | Best Ask %s | Spread %s",
                fmt_px(best_bid).c_str(), fmt_px(best_ask).c_str(), fmt_px(spread).c_str());
        attroff(COLOR_PAIR(3) | A_BOLD);
    }
}

static void render_recent_trades(const std::vector<Trade>& trades, int trade_start, int rows) 
{
    if (trade_start < rows - 2) 
    {
        mvhline(trade_start - 1, 1, '-', 50);
        mvprintw(trade_start, 2, "Recent Trades (Qty @ Price) time");
        for (size_t i = 0; i < trades.size() && trade_start + 1 + (int)i < rows - 1; ++i) {
            const auto& trade = trades[trades.size() - 1 - i];
            int color = (i % 2 == 0) ? 1 : 2;
            attron(COLOR_PAIR(color));
            mvprintw(trade_start + 1 + (int)i, 2, "%8s @ %10s  %s",
                    fmt_qty(trade.quantity).c_str(), fmt_px(trade.price).c_str(), fmt_time(trade.timestamp).c_str());
            attroff(COLOR_PAIR(color));
        }
    }
}

// -- API
void init_cli() 
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_GREEN, -1); // bids
        init_pair(2, COLOR_RED,   -1); // asks
        init_pair(3, COLOR_YELLOW,-1); // spread/header
    }
}

void shutdown_cli() 
{
    endwin();
}

void render_market_dashboard(std::shared_ptr<MatchingEngine> matching_engine, size_t depth, size_t lastTrades, std::atomic<bool>& running) 
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int asks_col = 20 + 4; // adds padding
    static size_t sym_idx = 0;
    static std::vector<std::string> symbols;

    auto order_book_for_all_symbols = matching_engine->get_order_book_for_all_symbols();

    // fill symbols if not done yet
    if (symbols.empty())
    {
        symbols.reserve(order_book_for_all_symbols.size());
        for (const auto& [symbol, _] : order_book_for_all_symbols) {
            symbols.push_back(symbol);
        }
    }

    sym_idx = sym_idx % symbols.size();
    const auto& symbol = symbols[sym_idx];

    render_symbol_header(symbol, depth, cols, asks_col);

    auto bids = matching_engine->get_top_bids(symbol, depth);
    auto asks = matching_engine->get_top_asks(symbol, depth);
    auto trades = matching_engine->get_recent_trades_for_symbol(symbol, lastTrades);
    size_t max_rows = std::max(bids.size(), asks.size());

    render_order_book_sides(bids, asks, depth, asks_col, max_rows, rows);

    render_best_bid_ask_spread(bids, asks, max_rows);

    int trade_start = 4 + (int)max_rows + 3; // 3 for header and spread
    render_recent_trades(trades, trade_start, rows);

    refresh();

    // input
    int ch = getch();
    switch (ch) {
        case 'q': case 'Q': running = false; break;
        case KEY_RIGHT: sym_idx = (sym_idx + 1) % symbols.size(); break;
        case KEY_LEFT:  sym_idx = (sym_idx + symbols.size() - 1) % symbols.size(); break;
        case '+': case '=': depth = std::min<size_t>(50, depth + 1); break;
        case '-': case '_': depth = depth > 1 ? depth - 1 : 1; break;
        default: break;
    }
}
