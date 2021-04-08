//
// Created by ivan on 08.04.2021.
//

#include <QScrollBar>

#include "BasicImageViewport.h"
#include "HandDragScrollArea.h"

void adjustScrollBar(QScrollBar *scrollBar, double factor) {
    int val = int(factor * scrollBar->value()
                  + ((factor - 1) * scrollBar->pageStep() / 2));
    scrollBar->setValue(val);
}

BasicImageViewport::BasicImageViewport(QWidget *parent) : imageLabel(new QLabel),
                                                          scrollArea(new HandDragScrollArea) {
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    scrollArea->setAlignment(Qt::AlignCenter);
}

void BasicImageViewport::setImage(const QImage &image) {
    imageLabel->setPixmap(QPixmap::fromImage(image));
    scrollArea->setVisible(true);
}

void BasicImageViewport::scaleImage(double factor) {
    if (factor == 1.0) {
        imageLabel->adjustSize();
    } else {
        imageLabel->resize(factor * imageLabel->pixmap().size());
        adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
        adjustScrollBar(scrollArea->verticalScrollBar(), factor);
    }
}

double BasicImageViewport::calcFitToViewport(int statusBarGap) const {
    const int deltaX = 2;
    const int deltaY = statusBarGap == 0 ? deltaX : statusBarGap - 5;
    QSize size = imageLabel->pixmap().size();
    double newScaleFactor = std::min((double) (scrollArea->width() - deltaX) / size.width(),
                                     (double) (scrollArea->height() - deltaY) / size.height());
    return std::min(1.0, newScaleFactor);
}
