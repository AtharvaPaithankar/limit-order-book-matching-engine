#pragma once
#include "OrderBook.h"
#include "TradeLog.h"

// The core matching loop: price-time priority, partial fills, resting of
// unmatched quantity. This is the piece that ties the heap + hashmap +
// linked list together into one system.
class MatchingEngine {
public:
    explicit MatchingEngine(OrderBook& book) : book_(book) {}

    // Submits a new incoming order. It is matched against the opposite
    // side first; any unfilled remainder is added ("rested") to the book.
    void submit(Order order);

    // Cancels a resting order. Returns false if the ID doesn't exist
    // (already filled, already cancelled, or never existed).
    bool cancel(int64_t orderId) { return book_.cancel(orderId); }

    const TradeLog& tradeLog() const { return log_; }

private:
    OrderBook& book_;
    TradeLog   log_;
    int64_t    nextTradeId_ = 1;

    void matchBuy(Order& incoming);
    void matchSell(Order& incoming);
    void restOrder(const Order& order);
};
