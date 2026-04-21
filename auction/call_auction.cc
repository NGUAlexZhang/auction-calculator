#include "call_auction.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>

CallAuction::CallAuction(OrderBook& order_book, bool start_processing) noexcept
    : Auction(order_book) {
  if (start_processing) {
    _processing_thread = std::jthread([this](std::stop_token stoken) {
      this->process_orders(stoken);
    });
  }
}

void CallAuction::execute_auction() {
  this->_order_book.event_queue().drain_for(std::chrono::milliseconds(0));
  this->handler_order();
}

AuctionResult CallAuction::result() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return {
      .match_price = _match_price,
      .match_volume = _match_volume,
      .buy_surplus = _buy_surplus,
      .sell_surplus = _sell_surplus,
      .has_match = _match_volume > 0,
  };
}

std::vector<Match> CallAuction::matches() const {
  this->ensure_matches_current();
  std::lock_guard<std::mutex> lock(_mutex);
  return _matches;
}

void CallAuction::process_orders(std::stop_token stoken) {
  while (!stoken.stop_requested()) {
    auto events =
        this->_order_book.event_queue().drain_for(std::chrono::milliseconds(100));
    if (events.empty()) {
      continue;
    }
    this->handler_order();
  }
}

std::vector<double> CallAuction::candidate_prices(
    const OrderBook::AggregatedSnapshot& snapshot) const {
  std::vector<double> prices;
  prices.reserve(snapshot.bids.size() + snapshot.asks.size());
  for (const auto& [price, _] : snapshot.bids) {
    prices.push_back(price);
  }
  for (const auto& [price, _] : snapshot.asks) {
    prices.push_back(price);
  }
  std::sort(prices.begin(), prices.end());
  prices.erase(std::unique(prices.begin(), prices.end()), prices.end());
  return prices;
}

CallAuction::PriceCandidate CallAuction::evaluate_price(
    const OrderBook::AggregatedSnapshot& snapshot, double price) const {
  uint64_t cumulative_bid_volume = 0;
  uint64_t cumulative_ask_volume = 0;

  for (const auto& [bid_price, volume] : snapshot.bids) {
    if (bid_price >= price) {
      cumulative_bid_volume += volume;
    }
  }
  for (const auto& [ask_price, volume] : snapshot.asks) {
    if (ask_price <= price) {
      cumulative_ask_volume += volume;
    }
  }

  return {
      .price = price,
      .match_volume = std::min(cumulative_bid_volume, cumulative_ask_volume),
      .remaining_volume =
          cumulative_bid_volume > cumulative_ask_volume
              ? cumulative_bid_volume - cumulative_ask_volume
              : cumulative_ask_volume - cumulative_bid_volume,
  };
}

std::vector<CallAuction::PrioritizedOrder> CallAuction::prioritized_buys(
    const OrderBook::Snapshot& snapshot, double match_price) const {
  std::vector<PrioritizedOrder> buys;
  for (const auto& [price, orders] : snapshot.bids) {
    if (price < match_price) {
      continue;
    }
    for (const auto& order : orders) {
      buys.push_back({.order = order, .remaining_size = order->size});
    }
  }
  std::sort(buys.begin(), buys.end(),
            [](const PrioritizedOrder& lhs, const PrioritizedOrder& rhs) {
              if (lhs.order->price != rhs.order->price) {
                return lhs.order->price > rhs.order->price;
              }
              if (lhs.order->datetime != rhs.order->datetime) {
                return lhs.order->datetime < rhs.order->datetime;
              }
              if (lhs.order->seq_no != rhs.order->seq_no) {
                return lhs.order->seq_no < rhs.order->seq_no;
              }
              return lhs.order->order_id < rhs.order->order_id;
            });
  return buys;
}

std::vector<CallAuction::PrioritizedOrder> CallAuction::prioritized_sells(
    const OrderBook::Snapshot& snapshot, double match_price) const {
  std::vector<PrioritizedOrder> sells;
  for (const auto& [price, orders] : snapshot.asks) {
    if (price > match_price) {
      continue;
    }
    for (const auto& order : orders) {
      sells.push_back({.order = order, .remaining_size = order->size});
    }
  }
  std::sort(sells.begin(), sells.end(),
            [](const PrioritizedOrder& lhs, const PrioritizedOrder& rhs) {
              if (lhs.order->price != rhs.order->price) {
                return lhs.order->price < rhs.order->price;
              }
              if (lhs.order->datetime != rhs.order->datetime) {
                return lhs.order->datetime < rhs.order->datetime;
              }
              if (lhs.order->seq_no != rhs.order->seq_no) {
                return lhs.order->seq_no < rhs.order->seq_no;
              }
              return lhs.order->order_id < rhs.order->order_id;
            });
  return sells;
}

void CallAuction::ensure_matches_current() const {
  double match_price = 0.0;
  uint64_t match_volume = 0;
  uint64_t match_state_version = 0;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_matches_dirty) {
      return;
    }
    if (_match_volume == 0) {
      _matches.clear();
      _matches_dirty = false;
      return;
    }
    match_price = _match_price;
    match_volume = _match_volume;
    match_state_version = _match_state_version;
  }

  this->recompute_matches(this->_order_book.full_snapshot(), match_price,
                          match_volume, match_state_version);

  std::lock_guard<std::mutex> lock(_mutex);
  if (_match_state_version != match_state_version) {
    _matches_dirty = true;
  }
}

void CallAuction::recompute_matches(const OrderBook::Snapshot& snapshot,
                                    double match_price,
                                    uint64_t match_volume,
                                    uint64_t match_state_version) const {
  std::vector<Match> matches;
  auto buys = this->prioritized_buys(snapshot, match_price);
  auto sells = this->prioritized_sells(snapshot, match_price);
  uint64_t remaining_match_volume = match_volume;
  size_t buy_index = 0;
  size_t sell_index = 0;

  while (remaining_match_volume > 0 && buy_index < buys.size() &&
         sell_index < sells.size()) {
    auto& buy = buys[buy_index];
    auto& sell = sells[sell_index];
    const auto quantity =
        std::min({buy.remaining_size, sell.remaining_size, remaining_match_volume});
    if (quantity == 0) {
      if (buy.remaining_size == 0) {
        ++buy_index;
      }
      if (sell.remaining_size == 0) {
        ++sell_index;
      }
      continue;
    }

    matches.push_back({
        .buy_order_id = buy.order->order_id,
        .sell_order_id = sell.order->order_id,
        .price = match_price,
        .quantity = quantity,
        .timestamp = std::max(buy.order->datetime, sell.order->datetime),
    });

    buy.remaining_size -= quantity;
    sell.remaining_size -= quantity;
    remaining_match_volume -= quantity;

    if (buy.remaining_size == 0) {
      ++buy_index;
    }
    if (sell.remaining_size == 0) {
      ++sell_index;
    }
  }

  std::unique_lock lock(_mutex);
  if (_match_state_version != match_state_version) {
    return;
  }
  _matches = std::move(matches);
  _matches_dirty = false;
}

void CallAuction::recompute_match_state(
    const OrderBook::AggregatedSnapshot& snapshot) {
  const auto prices = this->candidate_prices(snapshot);
  if (prices.empty()) {
    std::unique_lock lock(_mutex);
    _match_price = 0.0;
    _match_volume = 0;
    _match_remaining_volume = 0;
    _buy_surplus = 0;
    _sell_surplus = 0;
    _match_price_lower_bound = 0.0;
    _match_price_upper_bound = 0.0;
    ++_match_state_version;
    _matches.clear();
    _matches_dirty = false;
    return;
  }

  uint64_t best_match_volume = 0;
  uint64_t best_remaining_volume = std::numeric_limits<uint64_t>::max();
  std::vector<double> best_prices;

  for (double price : prices) {
    const auto candidate = this->evaluate_price(snapshot, price);
    if (candidate.match_volume == 0) {
      continue;
    }
    if (candidate.match_volume > best_match_volume) {
      best_match_volume = candidate.match_volume;
      best_remaining_volume = candidate.remaining_volume;
      best_prices = {candidate.price};
      continue;
    }
    if (candidate.match_volume == best_match_volume) {
      if (candidate.remaining_volume < best_remaining_volume) {
        best_remaining_volume = candidate.remaining_volume;
        best_prices = {candidate.price};
        continue;
      }
      if (candidate.remaining_volume == best_remaining_volume) {
        best_prices.push_back(candidate.price);
      }
    }
  }

  std::unique_lock lock(_mutex);
  if (best_prices.empty()) {
    _match_price = 0.0;
    _match_volume = 0;
    _match_remaining_volume = 0;
    _buy_surplus = 0;
    _sell_surplus = 0;
    _match_price_lower_bound = 0.0;
    _match_price_upper_bound = 0.0;
    ++_match_state_version;
    _matches.clear();
    _matches_dirty = false;
    return;
  }

  const auto [min_price_it, max_price_it] =
      std::minmax_element(best_prices.begin(), best_prices.end());
  _match_volume = best_match_volume;
  _match_remaining_volume = best_remaining_volume;
  _match_price_lower_bound = *min_price_it;
  _match_price_upper_bound = *max_price_it;
  _match_price = (_match_price_lower_bound + _match_price_upper_bound) / 2.0;

  _buy_surplus = 0;
  _sell_surplus = 0;
  for (const auto& [bid_price, volume] : snapshot.bids) {
    if (bid_price >= _match_price) {
      _buy_surplus += volume;
    }
  }
  for (const auto& [ask_price, volume] : snapshot.asks) {
    if (ask_price <= _match_price) {
      _sell_surplus += volume;
    }
  }
  _buy_surplus -= _match_volume;
  _sell_surplus -= _match_volume;
  ++_match_state_version;
  _matches_dirty = true;
}


void CallAuction::handler_order() {
  this->recompute_match_state(this->_order_book.aggregated_snapshot());
}
