#pragma once
#include "order_book.h"

class CallAuction {
 public:
  explicit CallAuction(OrderBook& order_book) noexcept;
  void execute_auction();
 private:
  OrderBook& order_book_;
};