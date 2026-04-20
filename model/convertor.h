#pragma once
#include "order.h"
#include "trade.h"

// Trade order_to_trade(const Order& order) {
//   Trade trade;
//   trade.datetime = order.datetime;
//   trade.sym = order.sym;
//   trade.price = order.price;
//   trade.size = order.size;
//   trade.bid_order_id = (order.side == 1) ? order.order_id : 0;
//   trade.ask_order_id = (order.side == -1) ? order.order_id : 0;
//   trade.trade_id = order.order_id;  // Using order ID as trade ID for simplicity
//   trade.exec_type = "NEW_ORDER";
//   trade.trade_bs_flag = (order.side == 1) ? "BUY" : "SELL";
//   trade.channel_no = order.channel_no;
//   trade.biz_index = order.biz_index;
//   return trade;
// }

inline Order trade_to_order(const Trade& trade) {
  Order order;
  order.datetime = trade.datetime;
  order.sym = trade.sym;
  order.price = trade.price;
  order.size = trade.size;
  if (trade.bid_order_id != 0) {
    order.side = 1;  // Buy
    order.order_id = trade.bid_order_id;
  } else if (trade.ask_order_id != 0) {
    order.side = -1; // Sell
    order.order_id = trade.ask_order_id;
  } else {
    throw std::runtime_error("Invalid trade: both bid and ask order IDs are zero");
  }
  order.order_type = 0; // Default to limit order for simplicity
  order.channel_no = trade.channel_no;
  order.biz_index = trade.biz_index;
  return order;
}
