#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "data_reader.h"
#include "data_simulator.h"
#include "order.h"
#include "trade.h"

TEST(OrderSimulatorTest, TestReadCSV) {
  std::string test_data_dir = TEST_DATA_DIR "order.csv";
  EXPECT_NO_THROW({
    DataSimulator<Order> simulator(test_data_dir);
    Order order;
    simulator >> order;
    simulator >> order;
  });
}

TEST(OrderParserTest, TestParsesTrailingFieldsInOrder) {
  std::istringstream line_stream(
      "2026-04-21 09:15:00,600000,10.25,100,1,2,123456,7,890,9999");

  Order order;
  ASSERT_NO_THROW(line_stream >> order);
  EXPECT_EQ(order.channel_no, 7U);
  EXPECT_EQ(order.seq_no, 890);
  EXPECT_EQ(order.biz_index, 9999);
}

TEST(OrderSimulatorTest, TestEndOfFile) {
  std::string test_data_dir = TEST_DATA_DIR "order.csv";

  EXPECT_NO_THROW({
    DataSimulator<Order> simulator(test_data_dir);
    Order order;
    while (simulator >> order) {
    }
  });
}

TEST(TradeSimulatorTest, TestReadCSV) {
  std::string test_data_dir = TEST_DATA_DIR "trade.csv";
  EXPECT_NO_THROW({
    DataSimulator<Trade> simulator(test_data_dir);
    Trade trade;
    simulator >> trade;
    simulator >> trade;
  });
}

TEST(TradeSimulatorTest, TestEndOfFile) {
  std::string test_data_dir = TEST_DATA_DIR "trade.csv";
  EXPECT_NO_THROW({
    DataSimulator<Trade> simulator(test_data_dir);
    Trade trade;
    while (simulator >> trade) {
      // std::cout << trade << std::endl;
    }
  });
};

TEST(DataReaderTest, TestReadTradeAndOrder) {
  std::string trade_data_dir = TEST_DATA_DIR "trade.csv";
  std::string order_data_dir = TEST_DATA_DIR "order.csv";
  EXPECT_NO_THROW({
    SafeQueue<Order> queue;
    std::atomic<bool> trade_finished = false;
    std::atomic<bool> order_finished = false;
    std::jthread order_thread(read_order, order_data_dir, std::ref(queue), std::ref(order_finished));
    std::jthread trade_thread(read_trade, trade_data_dir, std::ref(queue), std::ref(trade_finished));
    while (!trade_finished.load() || !order_finished.load()) {
      std::optional<Order> order;
      while (order = queue.try_pop()) {
        if(order == std::nullopt) {
          break;
        }
      }
    }
  });
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
