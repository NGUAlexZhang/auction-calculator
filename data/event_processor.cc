#include "event_processor.h"

#include <algorithm>

EventProcessor::EventProcessor(OrderBook& order_book, Auction& auction) noexcept
    : _order_book(order_book), _auction(auction) {}

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

void EventProcessor::apply_event(const MarketEvent& event) {
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
