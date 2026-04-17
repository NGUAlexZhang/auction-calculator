#include "data_reader.h"

void read_trade(std::stop_token st, const std::filesystem::path& file_path, SafeQueue<Order>& queue) {
  DataSimulator<Trade> simulator(file_path);
  Trade trade;
  while (!st.stop_requested() && (simulator >> trade)) {
    if (trade.price != 0) {
      continue;
    }
    queue.push(trade_to_order(trade));
  }
}

void read_order(std::stop_token st, const std::filesystem::path& file_path, SafeQueue<Order>& queue) {
  DataSimulator<Order> simulator(file_path);
  Order order;
  while (!st.stop_requested() && (simulator >> order)) {
    queue.push(order);
  }
}