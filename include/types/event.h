#pragma once
#include <cstdint>
#include <string>

enum class EventType {
  NEW_ORDER,  // New order event
  TRADE,      // Trade event
  CANCEL      // Order cancellation event
};

struct Event {
  int64_t biz_index;  // Business index for the event
  EventType type;     // Type of the event (NEW_ORDER, TRADE, CANCEL)
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
};
