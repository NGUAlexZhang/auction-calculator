#pragma once

#include <chrono>
#include <cstdint>

struct Tick {
  uint64_t volume;          // Number of shares traded in the tick
  double turnover;          // Total value of shares traded in the tick
  uint64_t total_bid_size;  // Total size of all bid orders in the tick
  uint64_t total_ask_size;  // Total size of all ask orders in the tick

  std::chrono::year_month_day date;                       // Date of the tick
  std::chrono::hh_mm_ss<std::chrono::milliseconds> time;  // Time
  std::string sym;                                        // Stock symbol
  double price;             // Price at which the tick occurred
  double prev_close_price;  // Previous closing price of the stock
  double open_price;        // Opening price of the stock
  double low_price;         // Lowest price of the stock during the trading day
  double iopv;  // Indicative Optimized Portfolio Value, used in ETF trading
  double high_price;       // Highest price of the stock during the trading day
  double close_price;      // Closing price of the stock
  double bid_prices[10];   // Array of bid prices at different levels
  double ask_prices[10];   // Array of ask prices at different levels
  uint64_t bid_sizes[10];  // Array of bid sizes corresponding to the bid prices
  uint64_t ask_sizes[10];  // Array of ask sizes corresponding to the ask prices
  double
      average_bid_price;  // Average price of the stock during the trading day
  double
      average_ask_price;  // Average price of the stock during the trading day
};