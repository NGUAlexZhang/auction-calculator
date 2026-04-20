#include "call_auction.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <iostream>
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
  auto orders = this->_order_book.order_queue().drain_for(std::chrono::milliseconds(0));
  for (const auto& order_ptr : orders) {
    if (order_ptr) {
      this->handler_order(order_ptr);
    }
  }
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

void CallAuction::process_orders(std::stop_token stoken) {
  while (!stoken.stop_requested()) {
    auto orders =
        this->_order_book.order_queue().drain_for(std::chrono::milliseconds(100));
    if (orders.empty()) {
      continue;
    }
    for (const auto& order_ptr : orders) {
      if (!order_ptr) {
        continue;
      }
      auto daypoint = std::chrono::floor<std::chrono::days>(order_ptr->datetime);
      auto timepoint = std::chrono::hh_mm_ss(order_ptr->datetime - daypoint);
      std::cout << "Processing order: " << order_ptr->order_id
                << ", Time: " << std::format("{:%H:%M:%S}", timepoint) << std::endl;
      this->handler_order(order_ptr);
    }
  }
}

std::vector<double> CallAuction::candidate_prices() const {
  std::vector<double> prices;
  prices.reserve(_price_bid_volume.size() + _price_ask_volume.size());
  for (const auto& [price, _] : _price_bid_volume) {
    prices.push_back(price);
  }
  for (const auto& [price, _] : _price_ask_volume) {
    prices.push_back(price);
  }
  std::sort(prices.begin(), prices.end());
  prices.erase(std::unique(prices.begin(), prices.end()), prices.end());
  return prices;
}

CallAuction::PriceCandidate CallAuction::evaluate_price(double price) const {
  uint64_t cumulative_bid_volume = 0;
  uint64_t cumulative_ask_volume = 0;

  for (const auto& [bid_price, volume] : _price_bid_volume) {
    if (bid_price >= price) {
      cumulative_bid_volume += volume;
    }
  }
  for (const auto& [ask_price, volume] : _price_ask_volume) {
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

void CallAuction::recompute_match_state() {
  const auto prices = this->candidate_prices();
  if (prices.empty()) {
    std::unique_lock lock(_mutex);
    _match_price = 0.0;
    _match_volume = 0;
    _match_remaining_volume = 0;
    _buy_surplus = 0;
    _sell_surplus = 0;
    _match_price_lower_bound = 0.0;
    _match_price_upper_bound = 0.0;
    return;
  }

  uint64_t best_match_volume = 0;
  uint64_t best_remaining_volume = std::numeric_limits<uint64_t>::max();
  std::vector<double> best_prices;

  for (double price : prices) {
    const auto candidate = this->evaluate_price(price);
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
  for (const auto& [bid_price, volume] : _price_bid_volume) {
    if (bid_price >= _match_price) {
      _buy_surplus += volume;
    }
  }
  for (const auto& [ask_price, volume] : _price_ask_volume) {
    if (ask_price <= _match_price) {
      _sell_surplus += volume;
    }
  }
  _buy_surplus -= _match_volume;
  _sell_surplus -= _match_volume;
}


void CallAuction::handler_order(const std::shared_ptr<Order>& order) {
  if (order->side == 1) {
    _price_bid_volume[order->price] += order->size;
  } else if (order->side == -1) {
    _price_ask_volume[order->price] += order->size;
  } else {
    return;
  }
  this->recompute_match_state();
}
