#pragma once
#define __DATA_READER_H__
#include <atomic>

#include "convertor.h"
#include "data_simulator.h"
#include "order.h"
#include "safe_queue.h"
#include "trade.h"

void read_trade(std::stop_token st, const std::filesystem::path& file_path,
                SafeQueue<Order>& queue, std::atomic<bool>& finished);

void read_order(std::stop_token st, const std::filesystem::path& file_path,
                SafeQueue<Order>& queue, std::atomic<bool>& finished);