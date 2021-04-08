//
// Created by ivan on 06.04.2021.
//

#ifndef IM_VIEW_IMAGEVIEWER_H
#define IM_VIEW_IMAGEVIEWER_H

#include <QMainWindow>
#include <QImage>
#include <QDirIterator>
#include <QSettings>
#include "AsyncFileIterator.h"
#include "ImageViewport.h"

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
    explicit ImageViewer(ImageViewport *viewport, QWidget *parent = nullptr);

    ~ImageViewer() override;

    bool loadFile(const QFileInfo &);

public slots:

    void loadImage(const QString &filePath);

private slots:

    void open();

    void saveAs();

    void deleteFile();

    void print();

    void copy();

    void paste();

    void zoomIn();

    void zoomOut();

    void normalSize();

    void fitToWindow();

    void about();

    void restoreSettings();

    void dropSettings();

    void saveGeometry();

    void saveSort();

    void saveScale();

    void savePath();

    void dropGeometry();

    void dropSort();

    void dropScale();

    void dropPath();

private:
    void restoreScale();

    void restoreGeometry();

    void restoreSort();

    static void restorePath();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

#ifdef Q_OS_MAC
    bool event(QEvent *event) override;
#endif
private:

    void loadNext();

    void loadPrevious();

    void createActions();

    void updateActions();

    bool saveFile(const QString &fileName);

    bool setImage(const QImage &newImage);

    void scaleImage(double factor);

    void sortByName();

    void sortByTime();

    void reverseSort();

    QDir::SortFlags sortFlags();

    void fullScreenMode();

    bool canZoom(double factor);

    QImage image;
    QSettings settings;
    AsyncFileIterator iterator;
    double scaleFactor = 1;

#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    QPrinter printer;
    QAction *printAct;
#endif
    ImageViewport *imageViewPort;
    QAction *sortReversedAct;
    QAction *sortByNameAct;
    QAction *sortByTimeAct;
    QAction *saveAsAct;
    QAction *copyAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
};

#endif //IM_VIEW_IMAGEVIEWER_H