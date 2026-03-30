#include "order_book.h"

OrderBook::OrderBook(const OrderBook& other) {
  // Deep copy of buy_orders
  for (const auto& [price, orders] : other.buy_orders) {
    for (const auto& order_ptr : orders) {
      add_order(*order_ptr);
    }
  }
  // Deep copy of sell_orders
  for (const auto& [price, orders] : other.sell_orders) {
    for (const auto& order_ptr : orders) {
      add_order(*order_ptr);
    }
  }
}

OrderBook::OrderBook(OrderBook&& other)
    : buy_orders(std::move(other.buy_orders)),
      sell_orders(std::move(other.sell_orders)),
      id_map(std::move(other.id_map)) {}

OrderBook& OrderBook::operator=(const OrderBook& other) {
  if (this == &other) {
    return *this;
  }
  this->buy_orders.clear();
  this->sell_orders.clear();
  this->id_map.clear();

  for (auto& [price, orders] : other.buy_orders) {
    for (auto& order_ptr : orders) {
      add_order(*order_ptr);
    }
  }
  for (auto& [price, orders] : other.sell_orders) {
    for (auto& order_ptr : orders) {
      add_order(*order_ptr);
    }
  }

  return *this;
}

OrderBook& OrderBook::operator=(OrderBook&& other) {
  if (this == &other) {
    return *this;
  }
  this->buy_orders = std::move(other.buy_orders);
  this->sell_orders = std::move(other.sell_orders);
  this->id_map = std::move(other.id_map);
  return *this;
}

void OrderBook::add_order(const Order& order) {
  auto it = id_map.find(order.order_id);
  if (it != id_map.end()) {
    throw std::runtime_error("Order ID already exists: " +
                             std::to_string(order.order_id));
  }
  if (order.side == 1) {
    buy_orders[order.price].push_back(std::make_unique<Order>(order));
    id_map[order.order_id] = {order.price, true,
                              std::prev(this->buy_orders[order.price].end())};
  } else {
    sell_orders[order.price].push_back(std::make_unique<Order>(order));
    id_map[order.order_id] = {order.price, false,
                              std::prev(this->sell_orders[order.price].end())};
  }
}

void OrderBook::remove_order(uint64_t order_id) {
  auto it = id_map.find(order_id);
  if (it == id_map.end()) {
    throw std::runtime_error("Order ID not found: " + std::to_string(order_id));
  }
  const auto& location = it->second;
  if (location.is_buy) {
    buy_orders[location.price].erase(location.it);
  } else {
    sell_orders[location.price].erase(location.it);
  }
  id_map.erase(it);
}