#include "data_reader.h"

void read_trade(std::stop_token st, const std::filesystem::path& file_path, SafeQueue<Order>& queue) {
  DataSimulator<Trade> simulator(file_path);
  while (simulator && !st.stop_requested()) {
    Trade trade;
    simulator >> trade;
    queue.push(trade_to_order(trade));
  }
}

void read_order(std::stop_token st, const std::filesystem::path& file_path, SafeQueue<Order>& queue) {
  DataSimulator<Order> simulator(file_path);
  while (simulator && !st.stop_requested()) {
    Order order;
    simulator >> order;
    queue.push(order);
  }
}