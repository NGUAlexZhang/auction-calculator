#include "order_book.h"

void OrderBook::add_order(const Order& order) {
    if (order.side == 1) {
        this->buy_orders[order.price].push_back(order);
    } else {
        this->sell_orders[order.price].push_back(order);
    }
    this->order_id_map[order.order_id] = order;
}