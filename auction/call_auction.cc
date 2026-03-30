#include "call_auction.h"

CallAuction::CallAuction(OrderBook& order_book) noexcept : order_book_(order_book) {
    this->execute_auction();
}

void CallAuction::execute_auction() {
    
}