//
// Created by ivan on 08.04.2021.
//

#include <QScrollBar>
#include <QGuiApplication>
#include <QClipboard>

#include "BasicImageViewport.h"
#include "HandDragScrollArea.h"

void adjustScrollBar(QScrollBar *scrollBar, double factor) {
    int val = int(factor * scrollBar->value()
                  + ((factor - 1) * scrollBar->pageStep() / 2));
    scrollBar->setValue(val);
}

BasicImageViewport::BasicImageViewport(const IdChecker<unsigned> &idChecker, QWidget *parent) : QObject(parent), idChecker(idChecker), imageLabel(new QLabel),
                                                          scrollArea(new HandDragScrollArea) {
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    scrollArea->setAlignment(Qt::AlignCenter);

    connect(this, &BasicImageViewport::newLabelPixmap, this, &BasicImageViewport::firstImageInit, Qt::SingleShotConnection);
    connect(this, &BasicImageViewport::newLabelPixmap, this, &BasicImageViewport::setLabelPixmap);
}

void BasicImageViewport::setImage(unsigned id, const QImage &image, bool fitToWindow) {
    if (idChecker.isCanceled(id))
        return;
    const QPixmap &pixmap = QPixmap::fromImage(image);
    emit newLabelPixmap(id, pixmap, fitToWindow);
}

void BasicImageViewport::scaleImage() {
    if (scaleFactor == 1.0) {
        imageLabel->adjustSize();
    } else {
        imageLabel->resize(scaleFactor * imageLabel->pixmap().size());
        adjustScrollBar(scrollArea->horizontalScrollBar(), scaleFactor);
        adjustScrollBar(scrollArea->verticalScrollBar(), scaleFactor);
    }
}

double BasicImageViewport::calcFitToViewport() const {
    const int deltaX = 2;
//    const int deltaY = statusBarGap == 0 ? deltaX : statusBarGap - 5;
    QSize size = imageLabel->pixmap().size();
    double newScaleFactor = std::min((double) (scrollArea->width() - deltaX) / size.width(),
                                     (double) (scrollArea->height() - deltaX) / size.height());
    return std::min(1.0, newScaleFactor);
}

void BasicImageViewport::setLabelPixmap(unsigned  id, const QPixmap &pixmap, bool fitToWindow) {
    if (idChecker.isCanceled(id))
        return;
    imageLabel->setPixmap(pixmap);
    this->fitToWindow(fitToWindow);
#ifdef Q_OS_MAC
    emit repaintRequired();
#endif
}

void BasicImageViewport::firstImageInit(unsigned  id, const QPixmap &pixmap, bool fitToWindow) {
    scrollArea->setVisible(true);
}

void BasicImageViewport::zoomIn() {
    if (canZoom(1.2)) {
        changeScale(1.2);
    }
}

void BasicImageViewport::zoomOut() {
    if (canZoom(0.8)) {
        changeScale(0.8);
    }
}

void BasicImageViewport::normalSize() {
    setScaleFactor(1.0);
    changeScale(1.0);
}

void BasicImageViewport::fitToWindow(bool value) {
    if (hasImage()) {
        if (value) {
            scaleFactor = calcFitToViewport();
        }
        scaleImage();
    }
}

void BasicImageViewport::changeScale(double factor) {
    if (hasImage()) {
        scaleFactor *= factor;
        scaleImage();
    }
}

const int MAX_IMAGE_SIZE = 20000 * 20000;

bool BasicImageViewport::canZoom(double factor) {
    const QSize &imSize = (getScaleFactor() * factor) * imageLabel->pixmap().size();
    return imSize.width() > 0 && imSize.height() > 0 && imSize.width() * imSize.height() < MAX_IMAGE_SIZE;
}

bool BasicImageViewport::hasImage() {
    return !imageLabel->pixmap().isNull();
}

void BasicImageViewport::copyToClipboard() {
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setPixmap(imageLabel->pixmap());
#endif // !QT_NO_CLIPBOARD
}

QImage BasicImageViewport::getImage() const {
    return imageLabel->pixmap().toImage();
}
