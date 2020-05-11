//
// Created by ivan on 11.05.2020.
//

#ifndef IMAGEVIEWER_OPENEVENTAPP_H
#define IMAGEVIEWER_OPENEVENTAPP_H

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
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            emit openRequest(openEvent->file());
        }

        return QApplication::event(event);
    }

    signals:
            void openRequest(QString);
};
#endif //IMAGEVIEWER_OPENEVENTAPP_H
