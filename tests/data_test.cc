#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "data_reader.h"
#include "data_simulator.h"
#include "order.h"
#include "trade.h"

std::ostream& operator<<(std::ostream& os, const Order& order) {
  os << "Datetime: " << std::format("{:%Y/%m/%d %H:%M:%S}", order.datetime)
     << ", Symbol: " << order.sym << ", Price: " << order.price
     << ", Size: " << order.size << ", Side: " << order.side
     << ", Order Type: " << order.order_type << ", Order ID: " << order.order_id
     << ", Channel No: " << order.channel_no << ", Seq No: " << order.seq_no
     << ", Biz Index: " << order.biz_index;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Trade& trade) {
  os << "Datetime: " << std::format("{:%Y/%m/%d %H:%M:%S}", trade.datetime)
     << ", Symbol: " << trade.sym << ", Price: " << trade.price
     << ", Size: " << trade.size << ", Bid Order ID: " << trade.bid_order_id
     << ", Ask Order ID: " << trade.ask_order_id
     << ", Trade ID: " << trade.trade_id << ", Exec Type: " << trade.exec_type
     << ", Trade BS Flag: " << trade.trade_bs_flag
     << ", Channel No: " << trade.channel_no
     << ", Biz Index: " << trade.biz_index;
  return os;
}

TEST(OrderSimulatorTest, TestReadCSV) {
  std::string test_data_dir = TEST_DATA_DIR "order.csv";
  EXPECT_NO_THROW({
    DataSimulator<Order> simulator(test_data_dir);
    Order order;
    simulator >> order;
    std::cout << order << std::endl;
    simulator >> order;
    std::cout << order << std::endl;
  });
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
    std::cout << trade << std::endl;
    simulator >> trade;
    std::cout << trade << std::endl;
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
    DataSimulator<Trade> trade_simulator(trade_data_dir);
    DataSimulator<Order> order_simulator(order_data_dir);
    SafeQueue<Order> queue;
    std::jthread trade_thread(read_trade, trade_data_dir, std::ref(queue));
    std::jthread order_thread(read_order, order_data_dir, std::ref(queue));
    while (!trade_thread.get_stop_token().stop_requested() &&
           !order_thread.get_stop_token().stop_requested()) {
      while (!queue.empty()) {
        auto order = queue.front();
        // if (order.price == 0) {
        //   std::cout << order << std::endl;
        // }
        queue.pop();
      }
    }
  });
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}