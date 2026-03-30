#pragma once
#include <map>
#include <unordered_map>
#include <memory>
#include <list>
#include "model/order.h"

class OrderBook {
public:
    OrderBook() noexcept = default;
    OrderBook(const OrderBook&) noexcept;
    OrderBook(OrderBook&&) noexcept;
    OrderBook& operator=(const OrderBook&) noexcept;
    OrderBook& operator=(OrderBook&&) noexcept;
    void add_order(const Order& order);
    void add_order(Order&& order);
    void remove_order(uint64_t order_id);

private:
    using OrderPtr = std::unique_ptr<Order>;
    struct OrderLocation {
        double price; // Price level of the order
        bool is_buy; // True if it's a buy order, false if it's a sell order
        std::list<OrderPtr>::iterator it; // Iterator to the order in the list
    };
    std::map<double, std::list<OrderPtr>> buy_orders;  // Buy orders sorted
    std::map<double, std::list<OrderPtr>> sell_orders; // Sell orders sorted
    std::unordered_map<uint64_t, OrderLocation> id_map; // Map from order ID to order location
};