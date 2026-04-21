#pragma once
#define __DATA_READER_H__
#include <atomic>

#include "event_processor.h"
#include "market_data_source_factory.h"
#include "convertor.h"
#include "data_simulator.h"
#include "order.h"
#include "safe_queue.h"
#include "trade.h"

void read_trade(std::stop_token st, const std::filesystem::path& file_path,
                SafeQueue<Order>& queue, std::atomic<bool>& finished);

void read_order(std::stop_token st, const std::filesystem::path& file_path,
                SafeQueue<Order>& queue, std::atomic<bool>& finished);

void replay_csv_data(const std::filesystem::path& order_path,
                     const std::filesystem::path& trade_path,
                     EventProcessor& processor);
