#pragma once
#include "auction.h"
#include "order_book.h"

#include <cstdint>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>

class CallAuction : public Auction {
 public:
  explicit CallAuction(OrderBook& order_book,
                       bool start_processing = true) noexcept;
  void execute_auction() override;
  [[nodiscard]] AuctionResult result() const;
 private:
  struct PriceCandidate {
    double price{0.0};
    uint64_t match_volume{0};
    uint64_t remaining_volume{0};
  };

  std::jthread _processing_thread;
  void process_orders(std::stop_token stoken);
  void handler_order(const std::shared_ptr<Order>& order);
  void recompute_match_state();
  [[nodiscard]] std::vector<double> candidate_prices() const;
  [[nodiscard]] PriceCandidate evaluate_price(double price) const;
  std::unordered_map<double, uint64_t> _price_bid_volume;
  std::unordered_map<double, uint64_t> _price_ask_volume;
  mutable std::mutex _mutex;
  double _match_price{0.0};
  uint64_t _match_volume{0};
  uint64_t _match_remaining_volume{0};
  uint64_t _buy_surplus{0};
  uint64_t _sell_surplus{0};
  double _match_price_lower_bound{0.0};
  double _match_price_upper_bound{0.0};
};
