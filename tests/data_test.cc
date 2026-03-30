#include <gtest/gtest.h>

#include <chrono>
#include <iostream>

#include "order_simulator.h"

std::ostream& operator<<(std::ostream& os, const Order& order) {
  os << "Datetime: " << std::format("{:%Y/%m/%d %H:%M:%S}", order.datetime)
     << ", Symbol: " << order.sym << ", Price: " << order.price
     << ", Size: " << order.size << ", Side: " << order.side
     << ", Order Type: " << order.order_type << ", Order ID: " << order.order_id
     << ", Channel No: " << order.channel_no << ", Seq No: " << order.seq_no
     << ", Biz Index: " << order.biz_index;
  return os;
}

TEST(OrderSimulatorTest, TestReadCSV) {
  std::string test_data_dir = TEST_DATA_DIR "order.csv";
  EXPECT_NO_THROW({
    OrderSimulator simulator(test_data_dir);
    Order order = simulator.next_order();
    std::cout << order << std::endl;
    order = simulator.next_order();
    std::cout << order << std::endl;
  });
}

TEST(OrderSimulatorTest, TestEndOfFile) {
  std::string test_data_dir = TEST_DATA_DIR "order.csv";
  EXPECT_THROW({
    OrderSimulator simulator(test_data_dir);
    while (true) {
      auto order = simulator.next_order();
    }
  }, std::runtime_error);
  
  EXPECT_NO_THROW({
    OrderSimulator simulator(test_data_dir);
    while(simulator.has_next()) {
      Order order = simulator.next_order();
    }
  });
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}