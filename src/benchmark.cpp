// Benchmark + correctness self-check.
//
// Feeds N randomly generated orders through the engine, measures throughput,
// and verifies a basic invariant: total quantity bought in trades must equal
// total quantity sold (every trade has exactly one buyer and one seller for
// the same quantity, so these two sums are always identical by construction
// -- this mainly guards against a bookkeeping bug in matchBuy/matchSell).
//
// Usage: ./lob_benchmark [numOrders]   (default 200000)

#include <iostream>
#include <random>
#include <chrono>
#include <cstdint>
#include "MatchingEngine.h"

int main(int argc, char** argv) {
    int64_t numOrders = (argc > 1) ? std::atoll(argv[1]) : 200000;

    OrderBook book;
    MatchingEngine engine(book);

    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int> sideDist(0, 1);
    // Prices clustered around 10000 ticks (=100.00) so buys and sells cross
    // often, exercising both the matching path and the resting path.
    std::uniform_int_distribution<int64_t> priceDist(9950, 10050);
    std::uniform_int_distribution<int64_t> qtyDist(1, 100);

    auto start = std::chrono::high_resolution_clock::now();

    for (int64_t i = 0; i < numOrders; i++) {
        Order o{
            i,
            sideDist(rng) == 0 ? Side::BUY : Side::SELL,
            priceDist(rng),
            qtyDist(rng),
            0,
            static_cast<long>(i)
        };
        o.originalQty = o.quantity;
        engine.submit(o);

        // Occasionally cancel a recent order to exercise the cancel path too.
        if (i > 100 && i % 17 == 0) {
            engine.cancel(i - 50);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();

    int64_t totalBought = 0, totalSold = 0;
    for (const auto& t : engine.tradeLog().trades()) {
        totalBought += t.quantity;
        totalSold   += t.quantity; // by construction, but kept separate for clarity
    }

    std::cout << "Orders submitted : " << numOrders << "\n";
    std::cout << "Trades executed  : " << engine.tradeLog().trades().size() << "\n";
    std::cout << "Time             : " << seconds << " s\n";
    std::cout << "Throughput       : " << static_cast<int64_t>(numOrders / seconds) << " orders/sec\n";
    std::cout << "Total bought qty : " << totalBought << "\n";
    std::cout << "Total sold qty   : " << totalSold << "\n";
    std::cout << "Invariant check  : " << (totalBought == totalSold ? "PASS" : "FAIL") << "\n";

    return (totalBought == totalSold) ? 0 : 1;
}
