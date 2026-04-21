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
    }
    order_books[first_order.sym].cancel_order(first_order.order_id);
  });
  EXPECT_ANY_THROW({
    order_books[first_order.sym].cancel_order(first_order.order_id);
  });
  
}

TEST(OrderBookTest, TestSnapshotReturnsVisibleLevels) {
  OrderBook order_book;
  Order buy_order{};
  buy_order.datetime = std::chrono::sys_days{std::chrono::year{2026} /
                                             std::chrono::April / 21};
  buy_order.sym = "600000.SH";
  buy_order.price = 10.10;
  buy_order.size = 100;
  buy_order.side = 1;
  buy_order.order_type = 2;
  buy_order.order_id = 1;
  buy_order.channel_no = 1;
  buy_order.seq_no = 1;
  buy_order.biz_index = 1;

  Order sell_order = buy_order;
  sell_order.price = 10.20;
  sell_order.side = -1;
  sell_order.order_id = 2;
  sell_order.seq_no = 2;
  sell_order.biz_index = 2;

  order_book.add_order(buy_order);
  order_book.add_order(sell_order);

  const auto snapshot = order_book.snapshot();
  ASSERT_EQ(snapshot.bids.size(), 1U);
  ASSERT_EQ(snapshot.asks.size(), 1U);
  EXPECT_EQ(snapshot.bids.begin()->first, 10.10);
  EXPECT_EQ(snapshot.asks.begin()->first, 10.20);
  ASSERT_EQ(snapshot.bids.begin()->second.size(), 1U);
  ASSERT_EQ(snapshot.asks.begin()->second.size(), 1U);
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
