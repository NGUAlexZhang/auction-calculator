#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

#include "order.h"
#include "trade.h"

struct MarketEvent {
  enum class Type {
    NewOrder,
    CancelOrder,
  };

  Type type;
  std::chrono::sys_time<std::chrono::milliseconds> datetime;
  int64_t seq_no{0};
  int64_t biz_index{0};
  uint64_t order_id{0};
  std::optional<Order> order;

  [[nodiscard]] bool operator<(const MarketEvent& other) const {
    if (datetime != other.datetime) {
      return datetime < other.datetime;
    }
    if (seq_no != other.seq_no) {
      return seq_no < other.seq_no;
    }
    if (biz_index != other.biz_index) {
      return biz_index < other.biz_index;
    }
    if (type != other.type) {
      return type < other.type;
    }
    return order_id < other.order_id;
  }
};

inline MarketEvent order_to_event(const Order& order) {
  return {
      .type = MarketEvent::Type::NewOrder,
      .datetime = order.datetime,
      .seq_no = order.seq_no,
      .biz_index = order.biz_index,
      .order_id = order.order_id,
      .order = order,
  };
}

inline std::optional<MarketEvent> trade_to_event(const Trade& trade) {
  if (trade.price != 0) {
    return std::nullopt;
  }

  uint64_t order_id = 0;
  if (trade.bid_order_id != 0) {
    order_id = trade.bid_order_id;
  } else if (trade.ask_order_id != 0) {
    order_id = trade.ask_order_id;
  } else {
    throw std::runtime_error(
        "Invalid cancel event: both bid and ask order IDs are zero");
  }

  return MarketEvent{
      .type = MarketEvent::Type::CancelOrder,
      .datetime = trade.datetime,
      .seq_no = 0,
      .biz_index = trade.biz_index,
      .order_id = order_id,
      .order = std::nullopt,
  };
}
