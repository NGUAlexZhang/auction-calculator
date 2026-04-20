#pragma once
#include <chrono>
#include <cstdint>
#include <string>
#include <sstream>
#include "trade.h"

struct Order {
  std::chrono::sys_time<std::chrono::milliseconds>
      datetime;     // Date and time when the order was placed
  std::string sym;  // Stock symbol
  double price;     // Price at which the order is placed
  uint64_t size;    // Number of shares in the order
  int16_t side;     // 1 for buy, -1 for sell at Shanghai Stock Exchange, 3 for
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

std::istream& operator >> (std::istream& is, Order& order) {
    auto next_value = [&is]() -> std::string {
        std::string value;
        if (!std::getline(is, value, ',')) {
            throw std::runtime_error("Failed to parse line stream");
        }
        return value;
    };
    auto datetime_str = next_value();
    std::istringstream datetime_ss(datetime_str);
    std::chrono::system_clock::time_point tp;
    std::chrono::from_stream(datetime_ss, "%Y-%m-%d %H:%M:%S", tp);
    order.datetime = std::chrono::time_point_cast<std::chrono::milliseconds>(tp);
    order.sym = next_value();
    order.price = std::stod(next_value());
    order.size = std::stoull(next_value());
    order.side = std::stoi(next_value());
    order.order_type = std::stoul(next_value());
    order.order_id = std::stoull(next_value());
    order.channel_no = std::stoul(next_value());
    order.seq_no = std::stoll(next_value());
    order.biz_index = std::stoll(next_value());
    return is;
}