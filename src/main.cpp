// Order Book Matching Engine -- command-line driver.
//
// Input format (one command per line):
//   ADD BUY|SELL <orderId> <price> <qty>
//   CANCEL <orderId>
//   # comment lines and blank lines are ignored
//
// Usage:
//   ./lob_engine sample_orders.txt
//   ./lob_engine < sample_orders.txt
//   ./lob_engine                (then type commands, Ctrl-D to end)

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>
#include "MatchingEngine.h"

static int64_t toTicks(double price) {
    return static_cast<int64_t>(price * 100.0 + (price >= 0 ? 0.5 : -0.5));
}

int main(int argc, char** argv) {
    std::istream* in = &std::cin;
    std::ifstream file;
    if (argc > 1) {
        file.open(argv[1]);
        if (!file) {
            std::cerr << "Could not open file: " << argv[1] << "\n";
            return 1;
        }
        in = &file;
    }

    OrderBook book;
    MatchingEngine engine(book);

    std::string line;
    long ts = 0;
    while (std::getline(*in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "ADD") {
            std::string sideStr;
            int64_t id, qty;
            double price;
            iss >> sideStr >> id >> price >> qty;
            if (!iss) { std::cerr << "Malformed ADD line: " << line << "\n"; continue; }

            Order o{ id, sideStr == "BUY" ? Side::BUY : Side::SELL,
                     toTicks(price), qty, qty, ts++ };
            std::cout << "ADD    " << sideStr << " id=" << id
                      << " price=" << price << " qty=" << qty << "\n";
            engine.submit(o);
        } else if (cmd == "CANCEL") {
            int64_t id;
            iss >> id;
            bool ok = engine.cancel(id);
            std::cout << "CANCEL id=" << id << (ok ? " -> removed\n" : " -> not found\n");
        } else {
            std::cerr << "Unknown command, skipping: " << line << "\n";
        }
    }

    std::cout << "\n--- Trade Log ---\n";
    engine.tradeLog().printAll();
    std::cout << "\nTotal trades: " << engine.tradeLog().trades().size() << "\n";
    return 0;
}
