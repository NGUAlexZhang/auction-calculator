#include <gtest/gtest.h>
#include <chrono>
#include <unordered_map>
#include "order_book.h"
#include "data_simulator.h"

TEST(OrderBookTest, TestAddAndRemoveOrder) {
  std::unordered_map<std::string, OrderBook> order_books;
  DataSimulator<Order> simulator(TEST_DATA_DIR "order.csv");
  Order first_order;
  EXPECT_NO_THROW({
    bool readed = false;
    Order order;
    while (simulator >> order) {
      if (!readed) {
        first_order = order;
        readed = true;
      }
      order_books[order.sym].add_order(order);
      std::cout << "Added order: " << order.order_id << " for symbol: " << order.sym
                << " at price: " << order.price << std::endl;
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