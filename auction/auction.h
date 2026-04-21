#pragma once
#include "order_book.h"

enum class AuctionPhase {
    ACCEPTING_ORDERS,
    CALCULATING,
    EXECUTING
};

struct AuctionResult {
    double match_price{0};
    uint64_t match_volume{0};
    uint64_t buy_surplus{0};
    uint64_t sell_surplus{0};
    bool has_match{false};
};

struct Match {
    uint64_t buy_order_id{0};
    uint64_t sell_order_id{0};
    double price{0};
    uint64_t quantity{0};
    std::chrono::system_clock::time_point timestamp;
};

template<typename T>
concept LockFreeQueue = requires(T q, const Order& order) {
    {q.enqueue(order)} -> std::same_as<void>;
    {q.dequeue()} -> std::same_as<std::optional<Order>>;
};

class Auction {
public:
    Auction(OrderBook& order_book) noexcept;
    virtual void execute_auction() = 0;
    [[nodiscard]] virtual AuctionResult result() const = 0;
    [[nodiscard]] virtual std::vector<Match> matches() const = 0;
    virtual ~Auction() = default;
protected:
    OrderBook& _order_book;
};
