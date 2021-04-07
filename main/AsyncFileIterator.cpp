//
// Created by ivan on 05.04.2021.
//

#include <QDirIterator>
#include <QtConcurrent/QtConcurrent>

#include "AsyncFileIterator.h"

void AsyncFileIterator::setFile(const QFileInfo &file) {
    stateFuture.cancel();
    if (state.sameDir(file.dir())) {
        if (!state.sameFile(file)) {
            FileIteratorState newState(state);
            state = FileIteratorState(state.getSortBy());
            stateFuture = QtConcurrent::run(AsyncFileIterator::locateFile, file, newState);
        }
    } else {
        QDir::SortFlags sort = state.getSortBy();
        state = FileIteratorState(sort);
        stateFuture = QtConcurrent::run(AsyncFileIterator::loadDir, iteratorFilters, file, sort);
    }
}

void AsyncFileIterator::setSortFlags(QDir::SortFlags sortBy) {
    waitStateFuture();
    if (state.getSortBy() != sortBy) {
        FileIteratorState newState(state);
        newState.setSortBy(sortBy);
        state = FileIteratorState(sortBy);
        stateFuture = QtConcurrent::run(AsyncFileIterator::sort, std::move(newState));
    }
}

void AsyncFileIterator::reloadDir() {
    stateFuture.cancel();
    stateFuture = QtConcurrent::run(AsyncFileIterator::loadDir, iteratorFilters, state.current(), state.getSortBy());
    state = FileIteratorState(state.getSortBy());
}

int AsyncFileIterator::directorySize() {
    waitStateFuture();
    return state.getFiles().size();
}

FileIteratorState AsyncFileIterator::loadDir(const QStringList &iteratorFilters, const QFileInfo &file, QDir::SortFlags sortBy) {
    QFileInfoList files(file.dir().entryInfoList(iteratorFilters, QDir::Files | QDir::Readable, sortBy));
    FileIteratorState result(files, sortBy);
    result.locate(file);
    return result;
}

void AsyncFileIterator::waitStateFuture() {
    if (!stateFuture.isCanceled()) {
        if (!stateFuture.isFinished()) {
            stateFuture.waitForFinished();
        }
        state = stateFuture.result();
        stateFuture.cancel();
    }
}

FileIteratorState AsyncFileIterator::sort(FileIteratorState iteratorState) {
    iteratorState.sort();
    return iteratorState;
}

FileIteratorState AsyncFileIterator::locateFile(const QFileInfo &file, FileIteratorState iteratorState) {
    iteratorState.locate(file);
    return iteratorState;
}

