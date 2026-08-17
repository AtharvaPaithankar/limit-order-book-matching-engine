#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>

// A minimal open-addressing hash map (linear probing) with int64_t keys,
// written from scratch instead of using std::unordered_map. This is what
// gives the engine O(1) average-case OrderID -> node lookup for cancels,
// and O(1) average-case price -> PriceLevel lookup for order insertion.
template <typename V>
class HashMap {
public:
    explicit HashMap(size_t initialCapacity = 16) {
        capacity_ = nextPow2(initialCapacity);
        table_.assign(capacity_, Slot{});
    }

    void insert(int64_t key, const V& value) {
        if ((size_ + 1) * 2 > capacity_) rehash(capacity_ * 2);
        size_t idx = findInsertSlot(key);
        if (!table_[idx].occupied) size_++;
        table_[idx] = Slot{ key, value, true, false };
    }

    bool erase(int64_t key) {
        size_t idx = probeFind(key);
        if (idx == npos) return false;
        table_[idx].occupied = false;
        table_[idx].tombstone = true;
        size_--;
        return true;
    }

    // Returns a pointer to the stored value, or nullptr if the key is absent.
    V* find(int64_t key) {
        size_t idx = probeFind(key);
        return idx == npos ? nullptr : &table_[idx].value;
    }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

private:
    struct Slot {
        int64_t key = 0;
        V value{};
        bool occupied = false;
        bool tombstone = false;
    };

    static constexpr size_t npos = static_cast<size_t>(-1);

    std::vector<Slot> table_;
    size_t capacity_;
    size_t size_ = 0;

    static size_t nextPow2(size_t n) {
        size_t p = 1;
        while (p < n) p <<= 1;
        return p;
    }

    size_t hash(int64_t key) const {
        // 64-bit mix (splitmix64 finalizer) to spread keys evenly.
        uint64_t h = static_cast<uint64_t>(key);
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccdULL;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53ULL;
        h ^= h >> 33;
        return static_cast<size_t>(h) & (capacity_ - 1);
    }

    // Slot to write `key` into: its existing slot if present, otherwise the
    // first empty/tombstoned slot found while probing.
    size_t findInsertSlot(int64_t key) {
        size_t idx = hash(key);
        size_t firstTombstone = npos;
        for (size_t probe = 0; probe < capacity_; probe++) {
            size_t i = (idx + probe) & (capacity_ - 1);
            if (table_[i].occupied && table_[i].key == key) return i;
            if (!table_[i].occupied) {
                if (table_[i].tombstone) {
                    if (firstTombstone == npos) firstTombstone = i;
                } else {
                    return firstTombstone != npos ? firstTombstone : i;
                }
            }
        }
        return firstTombstone; // pathological: table full of tombstones
    }

    size_t probeFind(int64_t key) const {
        size_t idx = hash(key);
        for (size_t probe = 0; probe < capacity_; probe++) {
            size_t i = (idx + probe) & (capacity_ - 1);
            if (!table_[i].occupied && !table_[i].tombstone) return npos; // genuinely never inserted
            if (table_[i].occupied && table_[i].key == key) return i;
        }
        return npos;
    }

    void rehash(size_t newCapacity) {
        std::vector<Slot> old = std::move(table_);
        capacity_ = newCapacity;
        table_.assign(capacity_, Slot{});
        size_ = 0;
        for (auto& s : old) {
            if (s.occupied) insert(s.key, s.value);
        }
    }
};
