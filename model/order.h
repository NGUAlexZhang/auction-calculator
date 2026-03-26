#pragma once
#include <chrono>
#include <cstdint>
#include <string>

struct Order {
  std::chrono::sys_time<std::chrono::seconds> datetime; // Date and time when the order was placed
  std::string sym;                                        // Stock symbol
  double price;   // Price at which the order is placed
  uint64_t size;  // Number of shares in the order
  int16_t side;   // 1 for buy, -1 for sell at Shanghai Stock Exchange, 3 for
                  // Borrow in, -3 for Borrow out at Shenzhen Stock Exchange
  uint16_t order_type;  // 0 is unused at Shanghai Stock Exchange, 1 for market
                        // order, 2 for limit order, 3 for best price order at
                        // Shenzhen Stock Exchange
  uint64_t order_id;    // Unique identifier for the order
  uint32_t channel_no;  // Channel number through which the order was placed
  int64_t seq_no;       // Sequence number of the order in the trading day
  int64_t biz_index;    // Business index for the order at Shanghai Stock
                        // Exchange, not used at Shenzhen Stock Exchange
};