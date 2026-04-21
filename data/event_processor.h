#pragma once

#include <unordered_set>
#include <vector>

#include "auction.h"
#include "market_event.h"

class EventProcessor {
 public:
  enum class AuctionWindow {
    None,
    AShareOpening,
    AShareClosing,
  };

  explicit EventProcessor(OrderBook& order_book, Auction& auction,
                          AuctionWindow auction_window = AuctionWindow::None)
      noexcept;

  void submit(MarketEvent event);
  void flush();

 private:
  [[nodiscard]] bool allows_event(const MarketEvent& event) const noexcept;
  void apply_event(const MarketEvent& event);

  OrderBook& _order_book;
  Auction& _auction;
  AuctionWindow _auction_window;
  std::vector<MarketEvent> _buffer;
  std::unordered_set<uint64_t> _pending_cancels;
};
