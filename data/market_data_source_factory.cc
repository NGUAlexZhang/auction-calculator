#include "market_data_source_factory.h"

#include <stdexcept>

#include "csv_market_data_source.h"

std::unique_ptr<IMarketDataSource> MarketDataSourceFactory::create(
    const MarketDataSourceConfig& config) {
  switch (config.type) {
    case MarketDataSourceType::Csv:
      return std::make_unique<CsvMarketDataSource>(config.order_path,
                                                   config.trade_path);
    case MarketDataSourceType::Network:
      throw std::runtime_error("NetworkMarketDataSource is not implemented yet");
  }
  throw std::runtime_error("Unsupported market data source type");
}
