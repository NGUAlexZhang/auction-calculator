#pragma once
#include <map>
#include <unordered_map>
#include "model/order.h"

class OrderBook {
public:
    OrderBook() = default;
    OrderBook(const OrderBook&);
    OrderBook& operator=(const OrderBook&);
    OrderBook(OrderBook&&) noexcept;
    OrderBook& operator=(OrderBook&&) noexcept;
    void add_order(const Order& order);
    void remove_order(uint64_t order_id);

private:
    std::map<double, std::vector<Order>> buy_orders;  // Buy orders sorted
    std::map<double, std::vector<Order>> sell_orders; // Sell orders sorted
    std::unordered_map<uint64_t, Order> order_id_map; // Map for quick order lookup by order_id
};