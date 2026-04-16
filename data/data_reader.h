#pragma once
#define __DATA_READER_H__
#include "data_simulator.h"
#include "safe_queue.h"
#include "order.h"
#include "trade.h"
#include "convertor.h"

void read_trade(std::stop_token st, const std::filesystem::path& file_path, SafeQueue<Order>& queue);

void read_order(std::stop_token st, const std::filesystem::path& file_path, SafeQueue<Order>& queue);