#pragma once
#include <cstdint>
#include "Order.h"

// Node in the FIFO linked list for one price level.
struct OrderNode {
    Order order;
    OrderNode* prev = nullptr;
    OrderNode* next = nullptr;
};

// All resting orders sitting at a single price, oldest first. A doubly
// linked list gives O(1) append (new order arrives), O(1) removal from the
// front (order fills, or matched away) and O(1) removal of an arbitrary
// order (a trader cancels something in the middle of the queue) -- an
// array/vector would need an O(n) shift for that last case.
class PriceLevel {
public:
    int64_t price;
    OrderNode* head = nullptr; // oldest order == next in line to be matched
    OrderNode* tail = nullptr; // newest order
    int64_t totalQuantity = 0;
    int      orderCount = 0;
    bool     inHeap = false;   // whether this level currently has a slot in the price heap

    explicit PriceLevel(int64_t p) : price(p) {}
    ~PriceLevel() {
        OrderNode* n = head;
        while (n) { OrderNode* nxt = n->next; delete n; n = nxt; }
    }
    PriceLevel(const PriceLevel&) = delete;
    PriceLevel& operator=(const PriceLevel&) = delete;

    OrderNode* pushBack(const Order& order) {
        OrderNode* node = new OrderNode{ order, tail, nullptr };
        if (tail) tail->next = node; else head = node;
        tail = node;
        totalQuantity += order.quantity;
        orderCount++;
        return node;
    }

    void remove(OrderNode* node) {
        if (node->prev) node->prev->next = node->next; else head = node->next;
        if (node->next) node->next->prev = node->prev; else tail = node->prev;
        totalQuantity -= node->order.quantity;
        orderCount--;
        delete node;
    }

    bool empty() const { return orderCount == 0; }
};
