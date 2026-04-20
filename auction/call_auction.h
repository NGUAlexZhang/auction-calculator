#pragma once
#include "auction.h"
#include "order_book.h"

#include <mutex>
#include <thread>
#include <unordered_map>

class CallAuction : public Auction {
 public:
  explicit CallAuction(OrderBook& order_book) noexcept;
  void execute_auction() override;
 private:
  std::jthread _processing_thread;
  void process_orders(std::stop_token stoken);
  void handler_order(const std::shared_ptr<Order>& order);
  std::unordered_map<double, uint64_t> _price_bid_volume;
  std::unordered_map<double, uint64_t> _price_ask_volume;
  mutable std::mutex _mutex;
  double _match_price{0.0};
  uint64_t _match_volume{0};
  uint64_t _match_remaining_volume{0};
  uint64_t _match_price_size{0};
};
