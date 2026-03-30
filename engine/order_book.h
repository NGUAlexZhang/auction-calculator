#pragma once
#include <list>
#include <map>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <functional>

#include "order.h"

class OrderBook {
 public:
  OrderBook() noexcept = default;
  OrderBook(const OrderBook&) noexcept;
  OrderBook(OrderBook&&) noexcept;
  OrderBook& operator=(const OrderBook&) noexcept;
  OrderBook& operator=(OrderBook&&) noexcept;
  size_t size() const noexcept;
  size_t buy_size() const noexcept;
  size_t sell_size() const noexcept;
  void add_order(const Order& order);
  void remove_order(uint64_t order_id);
  void register_on_order_changes(std::function<void(const Order&)> callback);

 private:
  using OrderPtr = std::unique_ptr<Order>;
  struct OrderLocation {
    double price;  // Price level of the order
    bool is_buy;   // True if it's a buy order, false if it's a sell order
    std::list<Order>::iterator it;  // Iterator to the order in the list
  };
  std::map<double, std::list<Order>> buy_orders;   // Buy orders sorted
  std::map<double, std::list<Order>> sell_orders;  // Sell orders sorted
  std::unordered_map<uint64_t, OrderLocation>
      id_map;                        // Map from order ID to order location
  mutable std::shared_mutex rw_mtx;  // Mutex for thread safety
  std::function<void(const Order&)> on_order_changes;  // Callback for new orders
};