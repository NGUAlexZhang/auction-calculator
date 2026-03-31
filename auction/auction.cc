#include "auction.h"

Auction::Auction(OrderBook& order_book) noexcept : _order_book(order_book) {
}