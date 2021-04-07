//
// Created by ivan on 05.04.2021.
//

#ifndef IM_VIEW_FILEITERATOR_H
#define IM_VIEW_FILEITERATOR_H

#include <QFileInfo>
#include <QDir>

class FileIterator {
public:
    virtual void setFile(const QFileInfo &file) = 0;
    virtual void setSortFlags(QDir::SortFlags sortBy) = 0;
    virtual void reloadDir() = 0;
    virtual int directorySize() = 0;
    virtual QFileInfo next() = 0;
    virtual QFileInfo previous() = 0;
    virtual QFileInfo current() const = 0;
    virtual void remove() = 0;
    virtual QFileInfo first() = 0;
    virtual QFileInfo last() = 0;
};

#endif //IM_VIEW_FILEITERATOR_H
