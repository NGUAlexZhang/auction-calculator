#pragma once

#include <filesystem>
#include <memory>

#include "market_data_source.h"

enum class MarketDataSourceType {
  Csv,
  Network,
};

struct MarketDataSourceConfig {
  MarketDataSourceType type{MarketDataSourceType::Csv};
  std::filesystem::path order_path;
  std::filesystem::path trade_path;
};

class MarketDataSourceFactory {
 public:
  static std::unique_ptr<IMarketDataSource> create(
      const MarketDataSourceConfig& config);
};
