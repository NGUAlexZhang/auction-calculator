#pragma once
#include "order_book.h"
#include "auction.h"
#include <atomic>
#include <thread>
#include <unordered_map>

class CallAuction : public Auction {
 public:
  explicit CallAuction(OrderBook& order_book) noexcept;
 private:
  std::jthread _processing_thread;
  void process_orders(std::stop_token stoken);
  void handler_order(const std::shared_ptr<Order>& order);
  std::unordered_map<double, uint64_t> _price_bid_volume;
  std::unordered_map<double, uint64_t> _price_ask_volume;
  mutable std::shared_mutex _mutex;
  double _match_price;
  uint64_t _match_volume;
  uint64_t _match_remaining_volume;
  uint64_t _match_price_size;
};