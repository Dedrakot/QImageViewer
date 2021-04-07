//
// Created by ivan on 06.04.2021.
//

#ifndef IM_VIEW_OPENEVENTAPP_H
#define IM_VIEW_OPENEVENTAPP_H
#include <QtWidgets/QApplication>
#include <QtGui/QFileOpenEvent>

class OpenEventApp: public QApplication {
Q_OBJECT
public:
    OpenEventApp(int &argc, char **argv)
            : QApplication(argc, argv)
    {
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen) {
            auto *openEvent = dynamic_cast<QFileOpenEvent *>(event);
            emit openRequest(openEvent->file());
        }

        return QApplication::event(event);
    }

signals:
    void openRequest(QString);
};
#endif //IM_VIEW_OPENEVENTAPP_H
