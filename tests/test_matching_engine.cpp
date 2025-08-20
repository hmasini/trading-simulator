#include "matching_engine.hpp"
#include "order.hpp"
#include "logger_mock.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class MatchingEngineTest : public ::testing::Test {
protected:
    std::shared_ptr<MockLogger> logger;

    void SetUp() override
    {
        logger = std::make_shared<MockLogger>();
    }
};

TEST_F(MatchingEngineTest, MatchSellOrder)
{
    // Given buy orders
    MatchingEngine engine(logger);

    EXPECT_CALL(*logger, log_order(testing::_, testing::_)).Times(5);

    engine.add_order({"NVDA", Side::BUY, 100.0, 10});
    engine.add_order({"NVDA", Side::BUY, 101.0, 5});
    // And one sell order
    engine.add_order({"NVDA", Side::SELL, 99.0, 10});

    // Then there is only one bid left for NVDA
    CentralLimitOrderBook& order_book = engine.get_order_book_by_symbol("NVDA");
    ASSERT_EQ(order_book.bids.size(), 1);
    ASSERT_EQ(order_book.bids.begin()->order_id, 1);
    ASSERT_EQ(order_book.bids.begin()->quantity, 5);
    ASSERT_EQ(order_book.bids.begin()->price, 100.0);
    ASSERT_EQ(engine.get_trades().size(), 2);
    ASSERT_EQ(order_book.asks.size(), 0);
    ASSERT_EQ(engine.get_trades().size(), 2);
}

TEST_F(MatchingEngineTest, MatchSellOrderTimePriority)
{
    // Given buy orders
    MatchingEngine engine(logger);
    EXPECT_CALL(*logger, log_order(testing::_, testing::_)).Times(7);

    engine.add_order({"NVDA", Side::BUY, 100.0, 4});
    engine.add_order({"NVDA", Side::BUY, 100.0, 5});
    engine.add_order({"NVDA", Side::BUY, 101.0, 5});
    // And one sell order
    engine.add_order({"NVDA", Side::SELL, 100.0, 10});

    // Then there is only one bid left for NVDA
    CentralLimitOrderBook& order_book = engine.get_order_book_by_symbol("NVDA");
    ASSERT_EQ(order_book.bids.size(), 1);
    ASSERT_EQ(order_book.bids.begin()->order_id, 2);
    ASSERT_EQ(order_book.bids.begin()->quantity, 4);
    ASSERT_EQ(order_book.bids.begin()->price, 100.0);
    ASSERT_EQ(order_book.asks.size(), 0);
    ASSERT_EQ(engine.get_trades().size(), 3);
}

TEST_F(MatchingEngineTest, NoMatchSellOrder)
{
    // Given buy orders
    MatchingEngine engine(logger);
    EXPECT_CALL(*logger, log_order(testing::_, testing::_)).Times(3);
    engine.add_order({"NVDA", Side::BUY, 100.0, 10});
    engine.add_order({"NVDA", Side::BUY, 101.0, 5});
    // And one sell order where there is no match
    engine.add_order({"NVDA", Side::SELL, 101.10, 10});

    // Then no match occurs
    CentralLimitOrderBook& order_book = engine.get_order_book_by_symbol("NVDA");
    ASSERT_EQ(order_book.bids.size(), 2);
    ASSERT_EQ(order_book.asks.size(), 1);
    ASSERT_EQ(engine.get_trades().size(), 0);
}

TEST_F(MatchingEngineTest, MatchBuyOrder)
{
    // Given two buy orders
    MatchingEngine engine(logger);
    EXPECT_CALL(*logger, log_order(testing::_, testing::_)).Times(5);
    engine.add_order({"NVDA", Side::SELL, 101.0, 10});
    engine.add_order({"NVDA", Side::SELL, 100.0, 5});
    // And one sell order
    engine.add_order({"NVDA", Side::BUY, 101.0, 10});

    // Then there is only one sell order left for NVDA
    CentralLimitOrderBook& order_book = engine.get_order_book_by_symbol("NVDA");
    ASSERT_EQ(order_book.asks.size(), 1);
    ASSERT_EQ(order_book.asks.begin()->order_id, 1);
    ASSERT_EQ(order_book.asks.begin()->quantity, 5);
    ASSERT_EQ(order_book.asks.begin()->price, 101.0);
    ASSERT_EQ(order_book.bids.size(), 0);
    ASSERT_EQ(engine.get_trades().size(), 2);
}

TEST_F(MatchingEngineTest, MatchBuyOrderTimePriority)
{
    // Given two buy orders
    MatchingEngine engine(logger);
    EXPECT_CALL(*logger, log_order(testing::_, testing::_)).Times(7);
    engine.add_order({"NVDA", Side::SELL, 101.0, 4});
    engine.add_order({"NVDA", Side::SELL, 101.0, 5});
    engine.add_order({"NVDA", Side::SELL, 100.0, 5});
    // And one sell order
    engine.add_order({"NVDA", Side::BUY, 101.0, 10});

    // Then there is only one sell order left for NVDA
    CentralLimitOrderBook& order_book = engine.get_order_book_by_symbol("NVDA");
    ASSERT_EQ(order_book.asks.size(), 1);
    ASSERT_EQ(order_book.asks.begin()->order_id, 2);
    ASSERT_EQ(order_book.asks.begin()->quantity, 4);
    ASSERT_EQ(order_book.asks.begin()->price, 101.0);
    ASSERT_EQ(order_book.bids.size(), 0);
    ASSERT_EQ(engine.get_trades().size(), 3);
}

TEST_F(MatchingEngineTest, NoMatchBuyOrder)
{
    // Given two buy orders
    MatchingEngine engine(logger);
    EXPECT_CALL(*logger, log_order(testing::_, testing::_)).Times(3);
    engine.add_order({"NVDA", Side::SELL, 101.0, 4});
    engine.add_order({"NVDA", Side::SELL, 100.0, 5});
    // And one sell order
    engine.add_order({"NVDA", Side::BUY, 99.5, 10});

    // Then no match occurs
    CentralLimitOrderBook& order_book = engine.get_order_book_by_symbol("NVDA");
    ASSERT_EQ(order_book.asks.size(), 2);
    ASSERT_EQ(order_book.bids.size(), 1);
    ASSERT_EQ(engine.get_trades().size(), 0);
}

TEST_F(MatchingEngineTest, CancelOrder)
{
    // Given a matching engine with two orders
    MatchingEngine engine(logger);
    EXPECT_CALL(*logger, log_order(testing::_, testing::_)).Times(3);
    engine.add_order({"NVDA", Side::BUY, 100.0, 10});
    engine.add_order({"NVDA", Side::SELL, 101.0, 5});

    // When an order is cancelled 
    ASSERT_TRUE(engine.cancel_order(1));

    // Check that the order was removed
    CentralLimitOrderBook& order_book = engine.get_order_book_by_symbol("NVDA");
    ASSERT_EQ(order_book.bids.size(), 0);
    ASSERT_EQ(order_book.asks.size(), 1);
    ASSERT_EQ(order_book.asks.begin()->order_id, 2);
}

TEST_F(MatchingEngineTest, TestAmendOrderQuantity)
{
    // Given a matching engine with two orders
    MatchingEngine engine(logger);
    EXPECT_CALL(*logger, log_order(testing::_, testing::_)).Times(2);
    engine.add_order({"NVDA", Side::BUY, 100.0, 10});

    // When an order is cancelled 
    ASSERT_TRUE(engine.amend_order(1, 100, 5));

    // Check that the order was amended
    CentralLimitOrderBook& order_book = engine.get_order_book_by_symbol("NVDA");
    ASSERT_EQ(order_book.bids.size(), 1);
    ASSERT_EQ(order_book.bids.begin()->order_id, 1);
    ASSERT_EQ(order_book.bids.begin()->quantity, 5);
    ASSERT_EQ(order_book.bids.begin()->price, 100);
    ASSERT_EQ(order_book.asks.size(), 0);
}

TEST_F(MatchingEngineTest, TestAmendedOrderPrice)
{
    // Given a matching engine with two orders
    MatchingEngine engine(logger);
    EXPECT_CALL(*logger, log_order(testing::_, testing::_)).Times(2);
    engine.add_order({"NVDA", Side::BUY, 101.0, 10});

    // When an order is cancelled 
    ASSERT_TRUE(engine.amend_order(1, 99, 10));

    // Check that the order was amended
    CentralLimitOrderBook& order_book = engine.get_order_book_by_symbol("NVDA");
    ASSERT_EQ(order_book.bids.size(), 1);
    ASSERT_EQ(order_book.bids.begin()->order_id, 1);
    ASSERT_EQ(order_book.bids.begin()->quantity, 10);
    ASSERT_EQ(order_book.bids.begin()->price, 99);
    ASSERT_EQ(order_book.asks.size(), 0);
}

