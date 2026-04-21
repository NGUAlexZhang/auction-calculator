#include <gtest/gtest.h>

#include "call_auction.h"
#include "event_processor.h"

namespace {

Order MakeOrder(uint64_t order_id, int16_t side, double price, uint64_t size,
                int64_t seq_no, int64_t biz_index) {
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
  order.seq_no = seq_no;
  order.biz_index = biz_index;
  return order;
}

}  // namespace

TEST(EventProcessorTest, HandlesCancelThatArrivesBeforeOrder) {
  OrderBook order_book;
  CallAuction auction(order_book, false);
  EventProcessor processor(order_book, auction);

  processor.submit({
      .type = MarketEvent::Type::CancelOrder,
      .datetime = std::chrono::sys_days{std::chrono::year{2026} /
                                        std::chrono::April / 21} +
                  std::chrono::milliseconds(1),
      .seq_no = 1,
      .biz_index = 1,
      .order_id = 1001,
      .order = std::nullopt,
  });
  processor.submit(order_to_event(MakeOrder(1001, 1, 10.10, 100, 2, 2)));
  processor.submit(order_to_event(MakeOrder(2001, -1, 10.00, 100, 3, 3)));

  processor.flush();

  EXPECT_EQ(order_book.size(), 1U);
  const auto result = auction.result();
  EXPECT_FALSE(result.has_match);
  EXPECT_EQ(result.match_volume, 0U);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
