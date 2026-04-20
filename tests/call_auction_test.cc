#include <gtest/gtest.h>

#include "call_auction.h"

namespace {

Order MakeOrder(uint64_t order_id, int16_t side, double price, uint64_t size) {
  Order order{};
  order.datetime = std::chrono::sys_days{std::chrono::year{2026} /
                                         std::chrono::April / 21} +
                   std::chrono::milliseconds(order_id);
  order.sym = "600000.SH";
  order.price = price;
  order.size = size;
  order.side = side;
  order.order_type = 2;
  order.order_id = order_id;
  order.channel_no = 1;
  order.seq_no = static_cast<int64_t>(order_id);
  order.biz_index = static_cast<int64_t>(order_id);
  return order;
}

}  // namespace

TEST(CallAuctionTest, SelectsAUnifiedAuctionPriceForCrossingOrders) {
  OrderBook order_book;
  order_book.add_order(MakeOrder(1, 1, 10.10, 100));
  order_book.add_order(MakeOrder(2, 1, 10.05, 200));
  order_book.add_order(MakeOrder(3, -1, 10.00, 150));
  order_book.add_order(MakeOrder(4, -1, 10.05, 100));

  CallAuction auction(order_book, false);
  auction.execute_auction();

  const auto result = auction.result();
  EXPECT_TRUE(result.has_match);
  EXPECT_DOUBLE_EQ(result.match_price, 10.05);
  EXPECT_EQ(result.match_volume, 250U);
  EXPECT_EQ(result.buy_surplus, 50U);
  EXPECT_EQ(result.sell_surplus, 0U);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
