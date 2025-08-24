#include "logger.hpp"
#include "order_types.hpp"
#include <fstream>

Logger::Logger(const std::string& filename)
    : m_filename(filename)
{
}

void Logger::log_order(EventType event, const std::string& message) 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_log_entries.size() >= MAX_LOG_ENTRIES) 
    {
        m_log_entries.pop_front(); // Remove oldest
    }
    m_log_entries.push_back({event, message});
}

void Logger::dump()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ofstream log_file(m_filename, std::ios::trunc);
    for (const auto& entry : m_log_entries) 
    {
        log_file << to_string(entry.event) << " , " << entry.message << "\n";
    }
    log_file.close();
}
