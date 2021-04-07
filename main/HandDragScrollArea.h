//
// Created by ivan on 06.04.2021.
//

#ifndef IM_VIEW_HANDDRAGSCROLLAREA_H
#define IM_VIEW_HANDDRAGSCROLLAREA_H


#include <QScrollArea>

class HandDragScrollArea: public QScrollArea {
private:
    QPoint m_ScrollStart;
    bool movable;
public:
    HandDragScrollArea();
protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;
};


#endif //IM_VIEW_HANDDRAGSCROLLAREA_H
