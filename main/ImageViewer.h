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
#include "IdChecker.h"

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
    explicit ImageViewer(ImageViewport &viewport, IdChecker<unsigned> &idChecker, QWidget *parent = nullptr);

    void loadFile(const QFileInfo &);

    void loadFileAsync(const QString &filePath, unsigned int loadId);

public slots:

    void loadImage(const QString &filePath);
#ifdef Q_OS_MAC
    void repaintForMac();
#endif

signals:
    void newStatus(const QString &message);

private slots:
    void showStatus(const QString &message);

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

    void keyReleaseEvent(QKeyEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

#ifdef Q_OS_MAC
    bool event(QEvent *event) override;
#endif
private:
    void initPrevious();

    void initNext();

    void load();

    void createActions();

    bool saveFile(const QString &fileName);

    void setImage(QImage &image, unsigned int i);

    void sortByName();

    void sortByTime();

    void reverseSort();

    QDir::SortFlags sortFlags();

    void fullScreenMode();

    void showNextFileStatus(const QFileInfo &f, const QString &addition);

    ImageViewport &imageViewPort;
    IdChecker<unsigned> &idChecker;
    QSettings settings;
    AsyncFileIterator iterator;
    QFuture<void> loadFuture;
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    QPrinter printer;
    QAction *printAct;
#endif
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
