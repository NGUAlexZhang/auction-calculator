#pragma once

#include <functional>

#include "market_event.h"

class IMarketDataSource {
 public:
  using EventHandler = std::function<void(MarketEvent)>;

  virtual ~IMarketDataSource() = default;
  virtual void set_handler(EventHandler handler) = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
};
