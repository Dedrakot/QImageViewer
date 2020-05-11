#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QImage>
#include <QDirIterator>
#include "ImageIterator.h"

#if defined(QT_PRINTSUPPORT_LIB)
#  include <QtPrintSupport/qtprintsupportglobal.h>

#  if QT_CONFIG(printer)
#    include <QPrinter>
#  endif
#endif

QT_BEGIN_NAMESPACE
class QAction;

class QLabel;

class QMenu;

class QScrollArea;

class QScrollBar;

QT_END_NAMESPACE

class ImageViewer : public QMainWindow {
Q_OBJECT

public:
    ImageViewer(QWidget *parent = nullptr);

    bool loadFile(const QFileInfo &);

public slots:
    void loadImage(const QString &filePath);
private slots:

    void open();

    void saveAs();

    void print();

    void copy();

    void paste();

    void zoomIn();

    void zoomOut();

    void normalSize();

    void fitToWindow();

    void about();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    bool isValid(const QString &typeName) const;

    void loadNext();

    void loadPrevious();

    void createActions();

    void updateActions();

    bool saveFile(const QString &fileName);

    void setImage(const QImage &newImage);

    void scaleImage(double factor);

    void sortByName();

    void sortByTime();

    void reverseSort();

    QImage image;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor = 1;

#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    QPrinter printer;
#endif

    QAction *sortReversedAct{};
    QAction *sortByNameAct{};
    QAction *sortByTimeAct{};
    QAction *saveAsAct{};
    QAction *printAct{};
    QAction *copyAct{};
    QAction *zoomInAct{};
    QAction *zoomOutAct{};
    QAction *normalSizeAct{};
    QAction *fitToWindowAct{};

    ImageIterator iterator;

    QDir::SortFlags sortOrder();

    void scalePixmap(double factor);

#ifndef Q_OS_MAC

    void fullScreenMode();

    QAction *fullScreenAct{};

#endif
};

#endif
