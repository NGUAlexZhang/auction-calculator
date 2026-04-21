#pragma once

#include <unordered_set>
#include <vector>

#include "auction.h"
#include "market_event.h"

class EventProcessor {
 public:
  explicit EventProcessor(OrderBook& order_book, Auction& auction) noexcept;

  void submit(MarketEvent event);
  void flush();

 private:
  void apply_event(const MarketEvent& event);

  OrderBook& _order_book;
  Auction& _auction;
  std::vector<MarketEvent> _buffer;
  std::unordered_set<uint64_t> _pending_cancels;
};
