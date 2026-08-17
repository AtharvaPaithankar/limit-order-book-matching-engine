#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <iomanip>

// One executed match. This is the only place a plain vector is used
// deliberately -- trades are never searched or cancelled, only appended and
// replayed in order, so a vector is the right (and simplest) tool.
struct Trade {
    int64_t tradeId;
    int64_t buyOrderId;
    int64_t sellOrderId;
    int64_t price;     // ticks
    int64_t quantity;
    long    timestamp;
};

class TradeLog {
public:
    void record(const Trade& t) { trades_.push_back(t); }

    void printAll() const {
        std::cout << std::left
                  << std::setw(9)  << "TradeID"
                  << std::setw(9)  << "BuyID"
                  << std::setw(9)  << "SellID"
                  << std::setw(10) << "Price"
                  << std::setw(9)  << "Qty"
                  << "Time\n";
        for (const auto& t : trades_) {
            std::cout << std::left
                      << std::setw(9)  << t.tradeId
                      << std::setw(9)  << t.buyOrderId
                      << std::setw(9)  << t.sellOrderId
                      << std::setw(10) << std::fixed << std::setprecision(2) << (t.price / 100.0)
                      << std::setw(9)  << t.quantity
                      << t.timestamp << "\n";
        }
    }

    const std::vector<Trade>& trades() const { return trades_; }

private:
    std::vector<Trade> trades_;
};
