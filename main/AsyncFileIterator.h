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

    virtual void setFile(const QFileInfo &file) override final;

    virtual void setSortFlags(QDir::SortFlags sortBy) override final;

    virtual void reloadDir() override final;

    virtual int directorySize() override final;

    virtual QFileInfo next() override final {
        waitStateFuture();
        return state.next();
    }

    virtual QFileInfo previous() override final {
        waitStateFuture();
        return state.previous();
    }

    virtual QFileInfo first() override final {
        waitStateFuture();
        state.goFront();
        return state.current();
    }

    virtual QFileInfo last() override final {
        waitStateFuture();
        state.goBack();
        return state.current();
    }

    virtual QFileInfo current() const final {
        return state.current();
    }

    virtual void remove() final {
        waitStateFuture();
        state.remove();
    }

    QDir::SortFlags sortFlags() {
        return state.getSortBy();
    }

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
