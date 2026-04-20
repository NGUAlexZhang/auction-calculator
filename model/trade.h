#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>


struct Trade {
  std::chrono::sys_time<std::chrono::milliseconds>
      datetime;     // Date and time when the event occurred
  std::string sym;  // Stock symbol
  double price;  // Price at which the event occurred (for NEW_ORDER and TRADE)
  uint64_t
      size;  // Number of shares involved in the event (for NEW_ORDER and TRADE)

  uint64_t bid_order_id;  // Unique identifier for the bid order in a trade
                          // event (for TRADE)
  uint64_t
      ask_order_id;  // Unique identifier for the ask order in a trade event
  uint64_t trade_id;  // Unique identifier for the trade event (for TRADE)
  std::string exec_type;  // Execution type for the trade event (for TRADE)
  std::string trade_bs_flag;  // Trade buy/sell flag for the trade event (for
                              // TRADE)
  uint32_t channel_no;  // Channel number through which the event was processed
  int64_t biz_index;  // Business index for the event

  bool operator==(const Trade& other) const {
    return this->trade_id == other.trade_id;
  }
  bool operator==(const std::uint64_t trade_id) const {
    return this->trade_id == trade_id;
  }

};

inline std::istream& operator >> (std::istream& is, Trade& trade) {
  auto next_value = [&is]() -> std::string {
    std::string value;
    if (!std::getline(is, value, ',')) {
      throw std::runtime_error("Failed to parse line stream");
    }
    return value;
  };
  std::istringstream datetime_ss(next_value());
  std::chrono::system_clock::time_point tp;
  std::chrono::from_stream(datetime_ss, "%Y-%m-%d %H:%M:%S", tp);
  trade.datetime = std::chrono::time_point_cast<std::chrono::milliseconds>(tp);
  trade.sym = next_value();
  trade.price = std::stod(next_value());
  trade.size = std::stoull(next_value());
  trade.bid_order_id = std::stoull(next_value());
  trade.ask_order_id = std::stoull(next_value());
  trade.trade_id = std::stoull(next_value());
  trade.exec_type = next_value();
  trade.trade_bs_flag = next_value();
  trade.channel_no = std::stoul(next_value());
  trade.biz_index = std::stoll(next_value());
  return is;
}
