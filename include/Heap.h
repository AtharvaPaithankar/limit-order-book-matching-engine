#pragma once
#include <vector>
#include <cstdint>
#include <utility>

// A minimal array-based binary heap, written from scratch instead of using
// std::priority_queue. Cmp(a, b) == true means "a has higher priority than
// b" (a should sit closer to the root).
//   - Bid side uses std::greater<int64_t>  -> max-heap -> highest price on top
//   - Ask side uses std::less<int64_t>     -> min-heap -> lowest price on top
// push/pop/top are all that's needed: the engine never has to change a
// price's priority once it's in the heap, only lazily discard stale entries
// (see OrderBook.h), so no decrease-key support is required.
template <typename Cmp>
class PriceHeap {
public:
    void push(int64_t price) {
        data_.push_back(price);
        siftUp(data_.size() - 1);
    }

    int64_t top() const { return data_.front(); }

    void pop() {
        data_.front() = data_.back();
        data_.pop_back();
        if (!data_.empty()) siftDown(0);
    }

    bool empty() const { return data_.empty(); }
    size_t size() const { return data_.size(); }

private:
    std::vector<int64_t> data_;
    Cmp cmp_;

    void siftUp(size_t i) {
        while (i > 0) {
            size_t parent = (i - 1) / 2;
            if (cmp_(data_[i], data_[parent])) {
                std::swap(data_[i], data_[parent]);
                i = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(size_t i) {
        size_t n = data_.size();
        while (true) {
            size_t l = 2 * i + 1, r = 2 * i + 2, best = i;
            if (l < n && cmp_(data_[l], data_[best])) best = l;
            if (r < n && cmp_(data_[r], data_[best])) best = r;
            if (best == i) break;
            std::swap(data_[i], data_[best]);
            i = best;
        }
    }
};
