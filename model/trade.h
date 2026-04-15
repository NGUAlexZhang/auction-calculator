#pragma once

#include <cstdint>
#include <string>
#include <iostream>

enum class TradeType {
  NEW_ORDER,  // New order event
  TRADE,      // Trade event
  CANCEL      // Order cancellation event
};

struct Trade {
  int64_t biz_index;  // Business index for the event
  TradeType type;     // Type of the event (NEW_ORDER, TRADE, CANCEL)
  uint64_t
      order_id;  // Unique identifier for the order associated with the event
  double price;  // Price at which the event occurred (for NEW_ORDER and TRADE)
  uint64_t
      size;  // Number of shares involved in the event (for NEW_ORDER and TRADE)
  int8_t side;  // 1 for buy, -1 for sell at Shanghai Stock Exchange, 3 for
                // Borrow in, -3 for Borrow out at Shenzhen Stock Exchange (for
                // NEW_ORDER and TRADE)
  uint64_t bid_order_id;  // Unique identifier for the bid order in a trade
                          // event (for TRADE)
  uint64_t
      ask_order_id;  // Unique identifier for the ask order in a trade event

  bool operator==(const Trade& other) const {
    return this->biz_index == other.biz_index && this->type == other.type &&
           this->order_id == other.order_id && this->price == other.price &&
           this->size == other.size && this->side == other.side &&
           this->bid_order_id == other.bid_order_id &&
           this->ask_order_id == other.ask_order_id;
  }
  bool operator==(const std::uint64_t order_id) const {
    return this->order_id == order_id;
  }

};