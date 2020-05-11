//
// Created by ivan on 05.05.2020.
//

#ifndef IMAGEVIEWER_IMAGEITERATOR_H
#define IMAGEVIEWER_IMAGEITERATOR_H

#include <QtCore/QString>
#include <QFileInfo>
#include <QDir>
#include <QFuture>
#include "IteratorState.h"

class ImageIterator {
public:

    void setFile(const QFileInfo &file);

    void setSortFlags(QDir::SortFlags sortBy);

    bool canIterate();

    QFileInfo next();

    QFileInfo previous();

    QFileInfo current() const;

    int directorySize() const;

    inline QDir::SortFlags getSortFlags() const {
        return state.getSortBy();
    }

    inline void removeCurrent() {
        state.remove();
    }

    inline QString getCurrentFile() const {
        return state.filePath();
    }

private:
    static IteratorState locateFile(const QFileInfo &file, IteratorState iteratorState);

    static IteratorState loadDir(const QFileInfo &file, QDir::SortFlags sortBy);

    static IteratorState sort(IteratorState iteratorState);

    void waitStateFuture();

    QFuture<IteratorState> stateFuture;
    IteratorState state;
};


#endif //IMAGEVIEWER_IMAGEITERATOR_H
