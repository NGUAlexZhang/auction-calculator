#pragma once

#include <filesystem>
#include <optional>
#include <thread>

#include "data_simulator.h"
#include "market_data_source.h"

class CsvMarketDataSource : public IMarketDataSource {
 public:
  CsvMarketDataSource(const std::filesystem::path& order_path,
                      const std::filesystem::path& trade_path);
  ~CsvMarketDataSource() override = default;

  void set_handler(EventHandler handler) override;
  void start() override;
  void stop() override;

 private:
  static std::optional<MarketEvent> next_order_event(
      DataSimulator<Order>& simulator);
  static std::optional<MarketEvent> next_trade_event(
      DataSimulator<Trade>& simulator);
  void emit_sorted_events();

  std::filesystem::path _order_path;
  std::filesystem::path _trade_path;
  EventHandler _handler;
  std::jthread _reader_thread;
};
