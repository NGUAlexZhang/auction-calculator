#include "order_book.h"
#include <mutex>


OrderBook::OrderBook(OrderBook&& other) noexcept {
  std::shared_lock lock(other._rw_mtx);  // Lock the other OrderBook for reading
  _bids = std::move(other._bids);
  _asks = std::move(other._asks);
  _id_map = std::move(other._id_map);
  _event_queue = std::move(other._event_queue);
}

OrderBook& OrderBook::operator=(OrderBook&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  std::unique_lock write_lock(
      _rw_mtx, std::defer_lock);  // Lock this OrderBook for writing
  std::shared_lock read_lock(
      other._rw_mtx, std::defer_lock);  // Lock the other OrderBook for reading
  std::lock(write_lock, read_lock);    // Lock both OrderBooks without deadlock
  this->_bids = std::move(other._bids);
  this->_asks = std::move(other._asks);
  this->_id_map = std::move(other._id_map);
  this->_event_queue = std::move(other._event_queue);
  return *this;
}


size_t OrderBook::size() const noexcept {
  std::shared_lock lock(_rw_mtx);  // Lock the OrderBook for reading
  return this->_id_map.size();
}

size_t OrderBook::buy_size() const noexcept {
  std::shared_lock lock(_rw_mtx);  // Lock the OrderBook for reading
  size_t count = 0;
  for (const auto& [price, orders] : this->_bids) {
    count += orders.size();
  }
  return count;
}

size_t OrderBook::sell_size() const noexcept {
  std::shared_lock lock(_rw_mtx);  // Lock the OrderBook for reading
  size_t count = 0;
  for (const auto& [price, orders] : this->_asks) {
    count += orders.size();
  }
  return count;
}



void OrderBook::add_order(const Order& order) {
  auto order_ptr = std::make_shared<Order>(order);
  {
    std::unique_lock lock(this->_rw_mtx);  // Lock the OrderBook for writing
    auto it = this->_id_map.find(order.order_id);
    if (it != this->_id_map.end()) {
      throw std::runtime_error("Order ID already exists: " +
                              std::to_string(order.order_id));
    }
    if (order.side == 1) {
      this->_bids[order.price].push_back(order_ptr);
      this->_id_map[order.order_id] = {order.price, true,
                                       std::prev(this->_bids[order.price].end())};
    } else {
      this->_asks[order.price].push_back(order_ptr);
      this->_id_map[order.order_id] = {order.price, false,
                                       std::prev(this->_asks[order.price].end())};
    }
  }
  this->_event_queue.push({EventType::Add, order.order_id});
}

void OrderBook::cancel_order(const uint64_t& order_id) {
  std::unique_lock lock(this->_rw_mtx);  // Lock the OrderBook for writing
  auto it = this->_id_map.find(order_id);
  if (it == this->_id_map.end()) {
    throw std::runtime_error("Order ID not found: " + std::to_string(order_id));
  }
  const auto& location = it->second;
  if (location.is_buy) {
    this->_bids[location.price].erase(location.it);
  } else {
    this->_asks[location.price].erase(location.it);
  }
  this->_id_map.erase(it);
  lock.unlock();
  this->_event_queue.push({EventType::Cancel, order_id});
}

OrderBook::Snapshot OrderBook::snapshot() const {
  return this->full_snapshot();
}

OrderBook::Snapshot OrderBook::full_snapshot() const {
  std::shared_lock lock(this->_rw_mtx);  // Lock the OrderBook for reading
  return {
      .bids = this->_bids,
      .asks = this->_asks,
  };
}

OrderBook::AggregatedSnapshot OrderBook::aggregated_snapshot() const {
  std::shared_lock lock(this->_rw_mtx);
  AggregatedSnapshot snapshot;

  for (const auto& [price, orders] : this->_bids) {
    uint64_t level_volume = 0;
    for (const auto& order : orders) {
      level_volume += order->size;
    }
    if (level_volume > 0) {
      snapshot.bids.emplace(price, level_volume);
    }
  }

  for (const auto& [price, orders] : this->_asks) {
    uint64_t level_volume = 0;
    for (const auto& order : orders) {
      level_volume += order->size;
    }
    if (level_volume > 0) {
      snapshot.asks.emplace(price, level_volume);
    }
  }

  return snapshot;
}

SafeQueue<OrderBook::Event>& OrderBook::event_queue() noexcept {
  return this->_event_queue;
}

double OrderBook::best_bid() const noexcept {
  std::shared_lock lock(this->_rw_mtx);  // Lock the OrderBook for reading
  if (this->_bids.empty()) {
    return 0.0;  // No buy orders, return 0 or some sentinel value
  }
  return this->_bids.rbegin()->first;  // The highest price in the buy orders
}

double OrderBook::best_ask() const noexcept {
  std::shared_lock lock(this->_rw_mtx);  // Lock the OrderBook for reading
  if (this->_asks.empty()) {
    return 0.0;  // No sell orders, return 0 or some sentinel value
  }
  return this->_asks.begin()->first;  // The lowest price in the sell orders
}
