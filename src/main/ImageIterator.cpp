//
// Created by ivan on 05.05.2020.
//

#include <QtCore/QMimeDatabase>
#include <QtGui/QImageReader>
#include <QDirIterator>
#include <QSet>
#include <QtConcurrent/QtConcurrent>

#include "ImageIterator.h"

QStringList defaultFilters();


const QStringList iteratorFilters = defaultFilters();

void ImageIterator::setFile(const QFileInfo &file) {
    if (state.sameDir(file.dir())) {
        if (!state.sameFile(file)) {
            IteratorState newState(state);
            state = IteratorState(state.getSortBy());
            stateFuture = QtConcurrent::run(ImageIterator::locateFile, file, newState);
        }
    } else {
        QDir::SortFlags sort = state.getSortBy();
        state = IteratorState(sort);
        stateFuture = QtConcurrent::run(ImageIterator::loadDir, file, sort);
    }

}

IteratorState ImageIterator::locateFile(const QFileInfo &file, IteratorState iteratorState) {
    iteratorState.locate(file);
    return iteratorState;
}

IteratorState ImageIterator::loadDir(const QFileInfo &file, QDir::SortFlags sortBy) {
    QFileInfoList files(file.dir().entryInfoList(iteratorFilters, QDir::Files | QDir::Readable, sortBy));
    IteratorState result(files, sortBy);
    result.locate(file);
    return result;
}

bool ImageIterator::canIterate() {
    waitStateFuture();
    return state.canIterate();
}

QFileInfo ImageIterator::next() {
    return state.next();
}

QFileInfo ImageIterator::previous() {
    return state.previous();
}

QStringList defaultFilters() {
    QStringList ret;
    QMimeDatabase db;
    for (const QByteArray &mime: QImageReader::supportedMimeTypes()) {
        const QMimeType &t = db.mimeTypeForName(mime);
        ret.append(t.globPatterns());
    }
    return ret;
}

int ImageIterator::knownSize() const {
    return state.getFiles().size();
}

void ImageIterator::waitStateFuture() {
    if (!stateFuture.isCanceled()) {
        if (!stateFuture.isFinished()) {
            stateFuture.waitForFinished();
        }
        state = stateFuture.result();
        stateFuture.cancel();
    }
}

void ImageIterator::setSort(QDir::SortFlags sortBy) {
    waitStateFuture();
    if (state.getSortBy() != sortBy) {
        IteratorState newState(state);
        newState.setSortBy(sortBy);
        state = IteratorState(sortBy);
        stateFuture = QtConcurrent::run(ImageIterator::sort, newState);
    }
}

IteratorState ImageIterator::sort(IteratorState iteratorState) {
    iteratorState.sort();
    return iteratorState;
}
