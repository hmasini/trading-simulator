#include "logger.hpp"
#include <gmock/gmock.h>

class MockLogger : public Logger {
public:
    MockLogger() : Logger("mock.log") {}
    MOCK_METHOD(void, log_order, (EventType, const std::string&), (override));
    MOCK_METHOD(void, dump, (), (override));
};