#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

#include "call_auction.h"
#include "data_reader.h"
#include "event_processor.h"
#include "market_data_source_factory.h"
#include "order_book.h"

namespace {

struct AppOptions {
  std::filesystem::path order_path;
  std::filesystem::path trade_path;
  size_t batch_size{500};
  std::chrono::milliseconds sleep_interval{20};
  bool show_progress{true};
  bool show_matches{false};
  std::optional<size_t> match_limit;
};

void print_usage(std::string_view program_name) {
  std::cerr << "Usage: " << program_name
            << " <order_csv_path> <trade_csv_path> [options]\n"
            << "Options:\n"
            << "  --batch-size <n>       Number of events per simulated batch "
               "(default: 500)\n"
            << "  --sleep-ms <ms>        Delay between batches in ms "
               "(default: 20)\n"
            << "  --show-progress        Print each batch result "
               "(default)\n"
            << "  --no-progress          Disable batch result printing\n"
            << "  --show-matches         Print matched orders at the end\n"
            << "  --no-matches           Skip matched order details "
               "(default)\n"
            << "  --match-limit <n>      Print at most n matched orders\n";
}

void print_auction_result(const AuctionResult& result, size_t processed_events) {
  std::cout << std::format(
      "[batch {}] has_match={}, price={:.4f}, volume={}, buy_surplus={}, "
      "sell_surplus={}\n",
      processed_events, result.has_match, result.match_price, result.match_volume,
      result.buy_surplus, result.sell_surplus);
}

AppOptions parse_options(int argc, char** argv) {
  if (argc < 3) {
    throw std::runtime_error("Missing required CSV paths");
  }

  AppOptions options{
      .order_path = argv[1],
      .trade_path = argv[2],
  };

  for (int i = 3; i < argc; ++i) {
    const std::string_view arg = argv[i];
    if (arg == "--batch-size") {
      if (i + 1 >= argc) {
        throw std::runtime_error("--batch-size requires a value");
      }
      options.batch_size = static_cast<size_t>(std::stoull(argv[++i]));
      continue;
    }
    if (arg == "--sleep-ms") {
      if (i + 1 >= argc) {
        throw std::runtime_error("--sleep-ms requires a value");
      }
      options.sleep_interval = std::chrono::milliseconds(std::stoi(argv[++i]));
      continue;
    }
    if (arg == "--show-progress") {
      options.show_progress = true;
      continue;
    }
    if (arg == "--no-progress") {
      options.show_progress = false;
      continue;
    }
    if (arg == "--show-matches") {
      options.show_matches = true;
      continue;
    }
    if (arg == "--no-matches") {
      options.show_matches = false;
      continue;
    }
    if (arg == "--match-limit") {
      if (i + 1 >= argc) {
        throw std::runtime_error("--match-limit requires a value");
      }
      options.match_limit = static_cast<size_t>(std::stoull(argv[++i]));
      continue;
    }
    throw std::runtime_error(std::format("Unknown argument: {}", arg));
  }

  if (options.batch_size == 0) {
    throw std::runtime_error("--batch-size must be greater than zero");
  }
  if (options.sleep_interval.count() < 0) {
    throw std::runtime_error("--sleep-ms must be non-negative");
  }

  return options;
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const auto options = parse_options(argc, argv);
    OrderBook order_book;
    CallAuction auction(order_book, false);
    EventProcessor processor(order_book, auction);
    auto source = MarketDataSourceFactory::create({
        .type = MarketDataSourceType::Csv,
        .order_path = options.order_path,
        .trade_path = options.trade_path,
    });

    size_t processed_events = 0;
    size_t buffered_events = 0;
    source->set_handler([&](MarketEvent event) {
      processor.submit(std::move(event));
      ++processed_events;
      ++buffered_events;
      if (buffered_events >= options.batch_size) {
        processor.flush();
        if (options.show_progress) {
          print_auction_result(auction.result(), processed_events);
        }
        buffered_events = 0;
        std::this_thread::sleep_for(options.sleep_interval);
      }
    });

    std::cout << std::format(
        "Starting simulated realtime call auction replay with batch_size={}, "
        "sleep={}ms, progress={}, show_matches={}\n",
        options.batch_size, options.sleep_interval.count(), options.show_progress,
        options.show_matches);
    source->start();
    source->stop();

    if (buffered_events > 0) {
      processor.flush();
      if (options.show_progress) {
        print_auction_result(auction.result(), processed_events);
      }
    }

    const auto result = auction.result();
    std::cout << "\nFinal auction result\n";
    std::cout << std::format(
        "has_match={}, price={:.4f}, volume={}, buy_surplus={}, "
        "sell_surplus={}\n",
        result.has_match, result.match_price, result.match_volume,
        result.buy_surplus, result.sell_surplus);

    const auto matches = auction.matches();
    std::cout << "\nMatched orders: " << matches.size() << "\n";
    if (options.show_matches) {
      const size_t limit = options.match_limit.value_or(matches.size());
      const size_t printed = std::min(limit, matches.size());
      for (size_t i = 0; i < printed; ++i) {
        const auto& match = matches[i];
        std::cout << std::format(
            "buy_order_id={}, sell_order_id={}, price={:.4f}, quantity={}, "
            "timestamp={:%Y-%m-%d %H:%M:%S}\n",
            match.buy_order_id, match.sell_order_id, match.price, match.quantity,
            match.timestamp);
      }
      if (printed < matches.size()) {
        std::cout << std::format("... {} more matches omitted\n",
                                 matches.size() - printed);
      }
    }
  } catch (const std::exception& ex) {
    print_usage(argv[0]);
    std::cerr << "Failed to run call auction pipeline: " << ex.what() << "\n";
    return 1;
  }

  return 0;
}
