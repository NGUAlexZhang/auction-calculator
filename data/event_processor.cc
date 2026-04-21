#include "event_processor.h"

#include <algorithm>

namespace {

std::chrono::milliseconds time_of_day(
    const std::chrono::sys_time<std::chrono::milliseconds>& timestamp) noexcept {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      timestamp - std::chrono::floor<std::chrono::days>(timestamp));
}

std::chrono::milliseconds hhmm(int hour, int minute, int second = 0) noexcept {
  return std::chrono::hours(hour) + std::chrono::minutes(minute) +
         std::chrono::seconds(second);
}

}  // namespace

EventProcessor::EventProcessor(OrderBook& order_book, Auction& auction,
                               AuctionWindow auction_window) noexcept
    : _order_book(order_book),
      _auction(auction),
      _auction_window(auction_window) {}

void EventProcessor::submit(MarketEvent event) {
  _buffer.push_back(std::move(event));
}

void EventProcessor::flush() {
  std::sort(_buffer.begin(), _buffer.end());
  for (const auto& event : _buffer) {
    apply_event(event);
  }
  _buffer.clear();
  _auction.execute_auction();
}

bool EventProcessor::allows_event(const MarketEvent& event) const noexcept {
  if (_auction_window == AuctionWindow::None) {
    return true;
  }

  const auto current_time = time_of_day(event.datetime);
  switch (_auction_window) {
    case AuctionWindow::None:
      return true;
    case AuctionWindow::AShareOpening:
      if (event.type == MarketEvent::Type::NewOrder) {
        return hhmm(9, 15) <= current_time && current_time <= hhmm(9, 25);
      }
      return hhmm(9, 15) <= current_time && current_time <= hhmm(9, 20);
    case AuctionWindow::AShareClosing:
      if (event.type == MarketEvent::Type::NewOrder) {
        return hhmm(14, 57) <= current_time && current_time <= hhmm(15, 0);
      }
      return false;
  }

  return false;
}

void EventProcessor::apply_event(const MarketEvent& event) {
  if (!allows_event(event)) {
    return;
  }

  switch (event.type) {
    case MarketEvent::Type::NewOrder:
      if (_pending_cancels.erase(event.order_id) > 0) {
        return;
      }
      if (event.order) {
        _order_book.add_order(*event.order);
      }
      return;
    case MarketEvent::Type::CancelOrder:
      try {
        _order_book.cancel_order(event.order_id);
      } catch (const std::runtime_error&) {
        _pending_cancels.insert(event.order_id);
      }
      return;
  }
}
