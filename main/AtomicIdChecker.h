//
// Created by ivan on 10.04.2021.
//

#ifndef IM_VIEW_ATOMICIDCHECKER_H
#define IM_VIEW_ATOMICIDCHECKER_H

#include <QAtomicInteger>

#include "IdChecker.h"

class AtomicIdChecker: public IdChecker<unsigned> {
private:
    QAtomicInteger<unsigned> counter;
public:
    AtomicIdChecker() : counter(0) {}

    unsigned int nextId() override {
        return ++counter;
    }

    [[nodiscard]] bool isCanceled(unsigned id) const final {
        return counter.loadAcquire() != id;
    }
};

#endif //IM_VIEW_ATOMICIDCHECKER_H
