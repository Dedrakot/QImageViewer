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

class QDirIterator;

class ImageIterator {
public:

    void setFile(const QFileInfo &file);

    void setSort(QDir::SortFlags sortBy);

    bool canIterate();

    QFileInfo next();

    QFileInfo previous();

    int knownSize() const;

    inline QDir::SortFlags getSortFlags() const {
        return state.getSortBy();
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
