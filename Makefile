CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Iinclude

.PHONY: all clean run bench

all: lob_engine lob_benchmark

lob_engine: src/main.cpp src/MatchingEngine.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

lob_benchmark: src/benchmark.cpp src/MatchingEngine.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

run: lob_engine
	./lob_engine sample_orders.txt

bench: lob_benchmark
	./lob_benchmark 200000

clean:
	rm -f lob_engine lob_benchmark
