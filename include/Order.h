#pragma once
#include <cstdint>

enum class Side { BUY, SELL };

// A single order. Price is stored in "ticks" (price * 100, i.e. paise/cents)
// as an integer instead of a double. This is deliberate: comparing floating
// point prices for equality (needed to find/merge price levels) is unsafe,
// so real exchanges quote prices as integer ticks. It's a small decision but
// a good one to be able to explain in an interview.
struct Order {
    int64_t id;
    Side    side;
    int64_t price;        // price in ticks (e.g. 10050 = 100.50)
    int64_t quantity;      // remaining (unfilled) quantity
    int64_t originalQty;   // quantity at time of submission, for reporting
    long    timestamp;     // monotonically increasing sequence number (arrival order)
};
