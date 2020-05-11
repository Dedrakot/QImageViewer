//
// Created by ivan on 09.05.2020.
//

#ifndef IMAGEVIEWER_ITERATORSTATE_H
#define IMAGEVIEWER_ITERATORSTATE_H

#include <QList>
#include <QDir>
#include <QFileInfo>
#include <utility>

class IteratorState {
public:
    IteratorState():sortBy(QDir::Name) {
    }

    explicit IteratorState(QDir::SortFlags sortBy):sortBy(sortBy) {
    }

    inline IteratorState(const IteratorState &other) = default;

    inline IteratorState(QFileInfoList files, QDir::SortFlags sortBy): files(std::move(files)), sortBy(sortBy) {
    }

    inline IteratorState &operator=(const IteratorState &other) = default;

    inline IteratorState(IteratorState &&other) noexcept:files(std::move(other.files)), sortBy(other.sortBy), uk(other.uk) {
    }

    inline IteratorState &operator=(IteratorState &&other) noexcept {
        files = std::move(other.files);
        sortBy = other.sortBy;
        uk = other.uk;
        return *this;
    }

    inline const QFileInfoList& getFiles() const {
        return files;
    }

    inline QDir::SortFlags getSortBy() const {
        return sortBy;
    }

    void locate(const QFileInfo &file);

    bool sameDir(const QDir &dir) const;

    bool sameFile(const QFileInfo &info);

    bool canIterate() const;

    QFileInfo next();

    QFileInfo previous();

    void sort();

    inline void setSortBy(QDir::SortFlags sortBy) {
        this->sortBy = sortBy;
    }

    void remove();

    QString filePath() const;

    QFileInfo current() const;
private:
    QFileInfoList files;
    QDir::SortFlags sortBy;
    int uk = 0;
};


#endif //IMAGEVIEWER_ITERATORSTATE_H
