//
// Created by ivan on 06.04.2021.
//

#include <QMouseEvent>
#include <QScrollBar>

#include "HandDragScrollArea.h"

HandDragScrollArea::HandDragScrollArea() {
//    horizontalScrollBar()->installEventFilter(this);
//    verticalScrollBar()->installEventFilter(this);
}

bool HandDragScrollArea::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Hide || event->type() == QEvent::Show) {
        if ((movable = horizontalScrollBar()->isVisible() || verticalScrollBar()->isVisible())) {
            widget()->setCursor(Qt::OpenHandCursor);
        } else {
            widget()->unsetCursor();
        }
    }
    return QScrollArea::eventFilter(obj, event);
}

void HandDragScrollArea::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_ScrollStart = event->pos();
        if (movable) {
            widget()->setCursor(Qt::ClosedHandCursor);
        }
    } else {
        QScrollArea::mousePressEvent(event);
    }
}

void moveScrollBar(int d, int viewPortSize, QScrollBar *scrollBar) {
    int delta = std::abs(d) * scrollBar->maximum() * 3 / viewPortSize / 2 + 1;
    scrollBar->setValue(scrollBar->value() + (d < 0 ? -delta : delta));
}

void HandDragScrollArea::mouseMoveEvent(QMouseEvent *event) {
    if (m_ScrollStart.isNull()) {
        QScrollArea::mouseMoveEvent(event);
    } else {
        QPoint pos = event->pos();
        const auto &viewportSize = viewport()->size();
        const auto &labelSize = widget()->size();
        QScrollBar *scrollBar = horizontalScrollBar();
        if (scrollBar->isVisible()) {
            int dx = m_ScrollStart.x() - pos.x();
            moveScrollBar(dx, viewportSize.width(), scrollBar);
        }
        scrollBar = verticalScrollBar();
        if (scrollBar->isVisible()) {
            int dy = m_ScrollStart.y() - pos.y();
            moveScrollBar(dy, viewportSize.height(), scrollBar);
        }
        m_ScrollStart = pos;
    }
}

void HandDragScrollArea::mouseReleaseEvent(QMouseEvent *event) {
    if (movable) {
        widget()->setCursor(Qt::OpenHandCursor);
    }
    m_ScrollStart = QPoint();
    QScrollArea::mouseReleaseEvent(event);
}