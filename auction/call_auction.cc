#include "call_auction.h"

#include <iostream>
#include <algorithm>


CallAuction::CallAuction(OrderBook& order_book) noexcept : Auction(order_book){
  _processing_thread = std::jthread([this](std::stop_token stoken) {
    this->process_orders(stoken);
  });
}

void CallAuction::process_orders(std::stop_token stoken) {
  while (!stoken.stop_requested()) {
    if (this->_order_book.size() == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Sleep briefly to avoid busy waiting
      continue;
    }
    auto order_ptr = this->_order_book.order_queue().front();
    if (order_ptr) {
      this->_order_book.order_queue().pop();
    }
    auto daypoint = std::chrono::floor<std::chrono::days>(order_ptr->datetime);
    auto timepoint = std::chrono::hh_mm_ss(order_ptr->datetime - daypoint);
    std::cout << "Processing order: " << order_ptr->order_id
              << ", Time: " << std::format("{:%H:%M:%S}", timepoint) << std::endl;
    this->handler_order(order_ptr);
  }
}


void CallAuction::handler_order(const std::shared_ptr<Order>& order) {
    if (order->side == 1) {
        _price_bid_volume[order->price] += order->size;
    } else {
        _price_ask_volume[order->price] += order->size;
    }
    auto bid_it = _price_bid_volume.find(order->price);
    auto ask_it = _price_ask_volume.find(order->price);
    if (bid_it != _price_bid_volume.end() && ask_it != _price_ask_volume.end()) {
        auto match_volume = std::min(bid_it->second, ask_it->second);
        auto match_remaining_volume = bid_it->second + ask_it->second - 2 * match_volume;
        std::unique_lock lock(_mutex);
        if (this->_match_volume == 0 || match_volume > this->_match_volume) {
            this->_match_price = order->price;
            this->_match_volume = match_volume;
            this->_match_remaining_volume = match_remaining_volume;
            this->_match_price_size = 1;
            return;
        }
        if (match_volume == this->_match_volume) {
            if(match_remaining_volume < this->_match_remaining_volume) {
                this->_match_price = order->price;
                this->_match_volume = match_volume;
                this->_match_remaining_volume = match_remaining_volume;
                this->_match_price_size = 1;
                return;
            }
            if (match_remaining_volume == this->_match_remaining_volume) {
                this->_match_price = (this->_match_price * this->_match_price_size + order->price) / (this->_match_price_size + 1);
                this->_match_price_size += 1;
            }
        }
    }
}