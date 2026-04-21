#pragma once
#include "auction.h"
#include "order_book.h"

#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

class CallAuction : public Auction {
 public:
  explicit CallAuction(OrderBook& order_book,
                       bool start_processing = true) noexcept;
  void execute_auction() override;
  [[nodiscard]] AuctionResult result() const override;
  [[nodiscard]] std::vector<Match> matches() const override;
 private:
  struct PriceCandidate {
    double price{0.0};
    uint64_t match_volume{0};
    uint64_t remaining_volume{0};
  };

  struct PrioritizedOrder {
    OrderBook::OrderPtr order;
    uint64_t remaining_size{0};
  };

  std::jthread _processing_thread;
  void process_orders(std::stop_token stoken);
  void handler_order();
  void recompute_match_state(const OrderBook::AggregatedSnapshot& snapshot);
  void ensure_matches_current() const;
  void recompute_matches(const OrderBook::Snapshot& snapshot, double match_price,
                         uint64_t match_volume,
                         uint64_t match_state_version) const;
  [[nodiscard]] std::vector<double> candidate_prices(
      const OrderBook::AggregatedSnapshot& snapshot) const;
  [[nodiscard]] PriceCandidate evaluate_price(
      const OrderBook::AggregatedSnapshot& snapshot, double price) const;
  [[nodiscard]] std::vector<PrioritizedOrder> prioritized_buys(
      const OrderBook::Snapshot& snapshot, double match_price) const;
  [[nodiscard]] std::vector<PrioritizedOrder> prioritized_sells(
      const OrderBook::Snapshot& snapshot, double match_price) const;
  mutable std::mutex _mutex;
  double _match_price{0.0};
  uint64_t _match_volume{0};
  uint64_t _match_remaining_volume{0};
  uint64_t _buy_surplus{0};
  uint64_t _sell_surplus{0};
  double _match_price_lower_bound{0.0};
  double _match_price_upper_bound{0.0};
  uint64_t _match_state_version{0};
  mutable std::vector<Match> _matches;
  mutable bool _matches_dirty{false};
};
