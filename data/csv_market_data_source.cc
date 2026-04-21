#include "csv_market_data_source.h"

#include <stdexcept>
#include <utility>

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
  _reader_thread = std::jthread(
      [this](std::stop_token stoken) { emit_merged_events(stoken); });
}

void CsvMarketDataSource::stop() {
  if (_reader_thread.joinable()) {
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

void CsvMarketDataSource::emit_merged_events(std::stop_token stoken) {
  DataSimulator<Order> order_simulator(_order_path);
  DataSimulator<Trade> trade_simulator(_trade_path);

  auto next_order = next_order_event(order_simulator);
  auto next_trade = next_trade_event(trade_simulator);

  while (!stoken.stop_requested() && (next_order || next_trade)) {
    const bool emit_order =
        next_order && (!next_trade || *next_order < *next_trade);
    if (emit_order) {
      _handler(std::move(*next_order));
      next_order = next_order_event(order_simulator);
      continue;
    }

    _handler(std::move(*next_trade));
    next_trade = next_trade_event(trade_simulator);
  }
}
