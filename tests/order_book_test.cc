#include <gtest/gtest.h>
#include <chrono>
#include <unordered_map>
#include "order_book.h"
#include "order_simulator.h"

TEST(OrderBookTest, TestAddAndRemoveOrder) {
  std::unordered_map<std::string, OrderBook> order_books;
  OrderSimulator simulator(TEST_DATA_DIR "order.csv");
  Order first_order;
  EXPECT_NO_THROW({
    bool readed = false;
    while (simulator.has_next()) {
      Order order = simulator.next_order();
      if (!readed) {
        first_order = order;
        readed = true;
      }
      order_books[order.sym].add_order(order);
    }
    order_books[first_order.sym].cancel_order(first_order.order_id);
  });
  EXPECT_ANY_THROW({
    order_books[first_order.sym].cancel_order(first_order.order_id);
  });
  
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}