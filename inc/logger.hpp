#pragma once

#include "order_types.hpp"

#include <deque>
#include <mutex>
#include <string>

struct LogEntry 
{
    EventType event;
    std::string message;
};

static constexpr size_t MAX_LOG_ENTRIES = 2000;

// Logger class for handling log entries
// Keeps a fixed-size log of the most recent entries
class Logger
{
public:
    Logger(const std::string& filename);
    virtual ~Logger() = default;

    // Logs events
    virtual void log_order(EventType event, const std::string& message);
    // Dump log entries to file
    virtual void dump();

private:
    std::string m_filename;
    std::deque<LogEntry> m_log_entries;

    std::mutex m_mutex;
};