//
// Created by ivan on 05.04.2021.
//

#ifndef IM_VIEW_FILEITERATORSTATE_H
#define IM_VIEW_FILEITERATORSTATE_H

#include <QList>
#include <QDir>
#include <QFileInfo>

class FileIteratorState {
    friend class AsyncFileIterator;

public:
    FileIteratorState() : FileIteratorState(QDir::Name) {}

    explicit FileIteratorState(QDir::SortFlags sortBy) : sortBy(sortBy) {}

    FileIteratorState(const FileIteratorState &other) = default;

    FileIteratorState &operator=(const FileIteratorState &other) = default;

    FileIteratorState(FileIteratorState &&other) noexcept: files(std::move(other.files)), sortBy(other.sortBy),
                                                           uk(other.uk) {}

    FileIteratorState &operator=(FileIteratorState &&other) noexcept {
        files = std::move(other.files);
        sortBy = other.sortBy;
        uk = other.uk;
        return *this;
    }

    inline FileIteratorState(QFileInfoList &&files, QDir::SortFlags sortBy) : files(std::move(files)), sortBy(sortBy) {}

    inline FileIteratorState(const QFileInfoList &files, QDir::SortFlags sortBy) : files(files), sortBy(sortBy) {}

private:

    inline const QFileInfoList &getFiles() const {
        return files;
    }

    inline QDir::SortFlags getSortBy() const {
        return sortBy;
    }

    void locate(const QFileInfo &file);

    bool sameDir(const QDir &dir) const;

    bool sameFile(const QFileInfo &info);

    QFileInfo next();

    QFileInfo previous();

    QFileInfo current() const;

    void goFront();

    void goBack();

    void sort();

    inline void setSortBy(QDir::SortFlags sortBy) {
        this->sortBy = sortBy;
    }

    void remove();
private:
    QFileInfoList files;
    QDir::SortFlags sortBy;
    int uk = 0;
};


#endif //IM_VIEW_FILEITERATORSTATE_H
