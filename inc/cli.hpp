#pragma once

#include <memory>
#include "matching_engine.hpp"

void init_cli();

void shutdown_cli();

void render_market_dashboard(std::shared_ptr<MatchingEngine> matching_engine, size_t depth, size_t lastTrades, std::atomic<bool>& running);

