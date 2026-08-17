#include "MatchingEngine.h"
#include <algorithm>

void MatchingEngine::submit(Order order) {
    if (order.side == Side::BUY) matchBuy(order);
    else                          matchSell(order);

    if (order.quantity > 0) restOrder(order); // whatever's left joins the book
}

void MatchingEngine::matchBuy(Order& incoming) {
    while (incoming.quantity > 0) {
        PriceLevel* best = book_.asks.bestLevel();
        if (!best || best->price > incoming.price) break; // nothing crosses

        OrderNode* resting = best->head; // oldest order at this price -> FIFO
        int64_t tradedQty = std::min(incoming.quantity, resting->order.quantity);

        // Execution price is the resting order's price -- it was already in
        // the book, so the incoming order "takes" that price.
        log_.record({ nextTradeId_++, incoming.id, resting->order.id,
                       best->price, tradedQty, incoming.timestamp });

        incoming.quantity       -= tradedQty;
        resting->order.quantity -= tradedQty;
        best->totalQuantity     -= tradedQty;

        if (resting->order.quantity == 0) {
            book_.locationIndex.erase(resting->order.id);
            best->remove(resting); // unlinks + frees the node
        }
    }
}

void MatchingEngine::matchSell(Order& incoming) {
    while (incoming.quantity > 0) {
        PriceLevel* best = book_.bids.bestLevel();
        if (!best || best->price < incoming.price) break;

        OrderNode* resting = best->head;
        int64_t tradedQty = std::min(incoming.quantity, resting->order.quantity);

        log_.record({ nextTradeId_++, resting->order.id, incoming.id,
                       best->price, tradedQty, incoming.timestamp });

        incoming.quantity       -= tradedQty;
        resting->order.quantity -= tradedQty;
        best->totalQuantity     -= tradedQty;

        if (resting->order.quantity == 0) {
            book_.locationIndex.erase(resting->order.id);
            best->remove(resting);
        }
    }
}

void MatchingEngine::restOrder(const Order& order) {
    PriceLevel* lvl = (order.side == Side::BUY)
                           ? book_.bids.getOrCreateLevel(order.price)
                           : book_.asks.getOrCreateLevel(order.price);
    OrderNode* node = lvl->pushBack(order);
    book_.locationIndex.insert(order.id, { node, order.side, order.price });
}
