//
// Created by ivan on 10.04.2021.
//

#ifndef IM_VIEW_IDCHECKER_H
#define IM_VIEW_IDCHECKER_H

template <typename T>
class IdChecker {
public:
    virtual T nextId() = 0;
    virtual bool isCanceled(T id) const;
};

#endif //IM_VIEW_IDCHECKER_H
