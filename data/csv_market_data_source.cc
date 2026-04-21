#include "csv_market_data_source.h"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

CsvMarketDataSource::CsvMarketDataSource(const std::filesystem::path& order_path,
                                         const std::filesystem::path& trade_path)
    : _order_path(order_path), _trade_path(trade_path) {}

void CsvMarketDataSource::set_handler(EventHandler handler) {
  _handler = std::move(handler);
}

void CsvMarketDataSource::start() {
  if (!_handler) {
    throw std::runtime_error("CsvMarketDataSource requires a handler before start");
  }
  _reader_thread = std::jthread([this](std::stop_token) { emit_sorted_events(); });
}

void CsvMarketDataSource::stop() {
  if (_reader_thread.joinable()) {
    _reader_thread.request_stop();
    _reader_thread.join();
  }
}

std::optional<MarketEvent> CsvMarketDataSource::next_order_event(
    DataSimulator<Order>& simulator) {
  Order order;
  if (!(simulator >> order)) {
    return std::nullopt;
  }
  return order_to_event(order);
}

std::optional<MarketEvent> CsvMarketDataSource::next_trade_event(
    DataSimulator<Trade>& simulator) {
  Trade trade;
  while (simulator >> trade) {
    if (auto event = trade_to_event(trade)) {
      return event;
    }
  }
  return std::nullopt;
}

void CsvMarketDataSource::emit_sorted_events() {
  DataSimulator<Order> order_simulator(_order_path);
  DataSimulator<Trade> trade_simulator(_trade_path);
  std::vector<MarketEvent> events;

  while (auto event = next_order_event(order_simulator)) {
    events.push_back(*event);
  }
  while (auto event = next_trade_event(trade_simulator)) {
    events.push_back(*event);
  }

  std::sort(events.begin(), events.end());
  for (auto& event : events) {
    _handler(std::move(event));
  }
}
