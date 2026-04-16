#pragma once
#include <list>
#include <map>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <functional>

#include "order.h"
#include "safe_queue.h"

class OrderBook {

public:
  using OrderPtr = std::shared_ptr<Order>;
  OrderBook() noexcept = default;
  OrderBook(const OrderBook&) noexcept = delete;
  OrderBook(OrderBook&&) noexcept;
  OrderBook& operator=(const OrderBook&) noexcept = delete;
  OrderBook& operator=(OrderBook&&) noexcept;
  void add_order(const Order& order);
  void cancel_order(const uint64_t& order_id);
  const std::map<double, std::list<OrderPtr>>& buy_orders() const noexcept;
  const std::map<double, std::list<OrderPtr>>& sell_orders() const noexcept;
  size_t size() const noexcept;
  size_t buy_size() const noexcept;
  size_t sell_size() const noexcept;
  SafeQueue<OrderPtr>& order_queue() noexcept;

  [[nodiscard]] double best_bid() const noexcept;

  [[nodiscard]] double best_ask() const noexcept;

 private:
  struct OrderLocation {
    double price;  // Price level of the order
    bool is_buy;   // True if it's a buy order, false if it's a sell order
    std::list<OrderPtr>::iterator it;  // Iterator to the order in the list
  };
  std::map<double, std::list<OrderPtr>> _bids;   // Buy orders sorted
  std::map<double, std::list<OrderPtr>> _asks;  // Sell orders sorted
  std::unordered_map<uint64_t, OrderLocation>
      _id_map;                        // Map from order ID to order location
  mutable std::shared_mutex _rw_mtx;  // Mutex for thread safety
  SafeQueue<OrderPtr> _order_queue;  // Queue for handling orders
};