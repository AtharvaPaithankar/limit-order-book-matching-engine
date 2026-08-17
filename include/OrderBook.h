#pragma once
#include <functional>
#include <vector>
#include <cstdint>
#include "Order.h"
#include "PriceLevel.h"
#include "Heap.h"
#include "HashMap.h"

// One side of the book (all bids, or all asks). Combines:
//   - index_ : priceTicks -> PriceLevel*   (O(1) "does this price level exist")
//   - heap_  : priority queue of priceTicks (O(log n) insert, O(1) peek best)
//
// Lazy deletion: when a price level empties out (last order filled or
// cancelled) it is NOT removed from the heap immediately -- that would need
// an O(log n) arbitrary-removal heap. Instead it's marked empty and simply
// left there; cleanTop() discards empty levels only when they surface at
// the top of the heap, which is the only place emptiness actually matters.
template <typename Cmp>
class BookSide {
public:
    ~BookSide() { for (auto* lvl : allLevels_) delete lvl; }
    BookSide() = default;
    BookSide(const BookSide&) = delete;
    BookSide& operator=(const BookSide&) = delete;

    PriceLevel* getOrCreateLevel(int64_t priceTicks) {
        PriceLevel** existing = index_.find(priceTicks);
        if (existing) {
            if (!(*existing)->inHeap) {
                heap_.push(priceTicks);
                (*existing)->inHeap = true;
            }
            return *existing;
        }
        PriceLevel* lvl = new PriceLevel(priceTicks);
        index_.insert(priceTicks, lvl);
        allLevels_.push_back(lvl);
        heap_.push(priceTicks);
        lvl->inHeap = true;
        return lvl;
    }

    void cleanTop() {
        while (!heap_.empty()) {
            PriceLevel** lvl = index_.find(heap_.top());
            if (lvl && !(*lvl)->empty()) return; // top is a real, non-empty level
            if (lvl) (*lvl)->inHeap = false;
            heap_.pop();
        }
    }

    bool empty() {
        cleanTop();
        return heap_.empty();
    }

    // Best price level (highest bid / lowest ask), or nullptr if book side is empty.
    PriceLevel* bestLevel() {
        cleanTop();
        if (heap_.empty()) return nullptr;
        return *index_.find(heap_.top());
    }

    PriceLevel* levelAt(int64_t priceTicks) {
        PriceLevel** lvl = index_.find(priceTicks);
        return lvl ? *lvl : nullptr;
    }

private:
    PriceHeap<Cmp> heap_;
    HashMap<PriceLevel*> index_;
    std::vector<PriceLevel*> allLevels_; // owns the PriceLevel objects
};

// The full book: bids (max-heap by price) + asks (min-heap by price),
// plus a global OrderID -> location index so any resting order can be
// cancelled in O(1) average time without scanning either side.
class OrderBook {
public:
    BookSide<std::greater<int64_t>> bids; // best bid = highest price
    BookSide<std::less<int64_t>>    asks; // best ask = lowest price

    struct Location { OrderNode* node; Side side; int64_t priceTicks; };
    HashMap<Location> locationIndex; // OrderID -> where it lives

    bool cancel(int64_t orderId) {
        Location* loc = locationIndex.find(orderId);
        if (!loc) return false;
        PriceLevel* lvl = (loc->side == Side::BUY) ? bids.levelAt(loc->priceTicks)
                                                     : asks.levelAt(loc->priceTicks);
        if (!lvl) return false;
        lvl->remove(loc->node);
        locationIndex.erase(orderId);
        return true;
    }
};
