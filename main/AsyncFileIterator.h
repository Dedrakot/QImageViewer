//
// Created by ivan on 05.04.2021.
//

#ifndef IM_VIEW_ASYNCFILEITERATOR_H
#define IM_VIEW_ASYNCFILEITERATOR_H

#include <QFuture>
#include <QStringList>

#include "FileIterator.h"
#include "FileIteratorState.h"

class AsyncFileIterator : public FileIterator {
public:
    AsyncFileIterator() {}

    explicit AsyncFileIterator(const QStringList &filters) : iteratorFilters(filters) {}

    explicit AsyncFileIterator(const QStringList &&filters) : iteratorFilters(std::move(filters)) {}

    void setFile(const QFileInfo &file) final;

    void setSortFlags(QDir::SortFlags sortBy) final;

    void reloadDir() final;

    int directorySize() final;

    int currentPosition() final;

    QFileInfo next() final {
        waitStateFuture();
        return state.next();
    }

    QFileInfo previous() final {
        waitStateFuture();
        return state.previous();
    }

    QFileInfo first() final {
        waitStateFuture();
        state.goFront();
        return state.current();
    }

    QFileInfo last() final {
        waitStateFuture();
        state.goBack();
        return state.current();
    }

    QFileInfo current() const final {
        return state.current();
    }

    void remove() final {
        waitStateFuture();
        state.remove();
    }

    QDir::SortFlags sortFlags() {
        return state.getSortBy();
    }

    bool isEmpty() final;

private:
    static FileIteratorState locateFile(const QFileInfo &file, FileIteratorState iteratorState);

    static FileIteratorState loadDir(const QStringList &iteratorFilters, const QFileInfo &file, QDir::SortFlags sortBy);

    static FileIteratorState sort(FileIteratorState iteratorState);

    void waitStateFuture();

    const QStringList iteratorFilters;
    QFuture<FileIteratorState> stateFuture;
    FileIteratorState state;
};


#endif //IM_VIEW_ASYNCFILEITERATOR_H
