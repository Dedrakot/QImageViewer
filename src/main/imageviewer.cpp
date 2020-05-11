#include "imageviewer.h"

#include <QApplication>
#include <QClipboard>
#include <QColorSpace>
#include <QDir>
#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QStandardPaths>
#include <QStatusBar>
#include <QKeyEvent>
#include <QDateTime>
#include <QtCore/QSettings>

#if defined(QT_PRINTSUPPORT_LIB)
#  include <QtPrintSupport/qtprintsupportglobal.h>

#  if QT_CONFIG(printdialog)
#    include <QPrintDialog>
#  endif
#endif

const QString SETTINGS_SORT = "view/sort";
const QString SETTINGS_GEOMETRY = "view/geometry";
const QString SETTINGS_SCALE = "view/scale";
//const QString SETTINGS_PATH = "view/path";

ImageViewer::ImageViewer(QWidget *parent) : QMainWindow(parent), imageLabel(new QLabel), scrollArea(new QScrollArea),
                                            settings("Dedrakot", "ImageViewer") {
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    scrollArea->setAlignment(Qt::AlignCenter);
    setCentralWidget(scrollArea);

    createActions();

    restoreSettings();
}

bool ImageViewer::loadFile(const QFileInfo &file) {
    iterator.setFile(file);
    QString filePath = file.filePath();
    QImageReader reader(filePath);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                         .arg(QDir::toNativeSeparators(filePath), reader.errorString()));
        return false;
    }

    setImage(newImage);

    setWindowFilePath(filePath);

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4, Modification time: %5")
            .arg(QDir::toNativeSeparators(filePath)).arg(image.width()).arg(image.height()).arg(image.depth()).arg(
            file.lastModified().toString());
    statusBar()->showMessage(message);
    return true;
}

void ImageViewer::setImage(const QImage &newImage) {
    image = newImage;
    if (image.colorSpace().isValid())
        image.convertToColorSpace(QColorSpace::SRgb);
    imageLabel->setPixmap(QPixmap::fromImage(image));

    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    updateActions();

    if (fitToWindowAct->isChecked()) {
        fitToWindow();
    } else if (scaleFactor != 1.0) {
        scalePixmap(scaleFactor);
    } else {
        imageLabel->adjustSize();
    }

#ifdef Q_OS_MAC
    const Qt::WindowStates &wState = windowState();
    if (wState.testFlag(Qt::WindowMaximized) || wState.testFlag(Qt::WindowFullScreen)) {
        repaint();
    }
#endif
}

bool ImageViewer::saveFile(const QString &fileName) {
    QImageWriter writer(fileName);

    if (!writer.write(image)) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                         .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode) {
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
                                              ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

void ImageViewer::open() {
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void ImageViewer::saveAs() {
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}

void ImageViewer::print() {
    Q_ASSERT(imageLabel->pixmap());
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printdialog)
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = imageLabel->pixmap()->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(imageLabel->pixmap()->rect());
        painter.drawPixmap(0, 0, *imageLabel->pixmap());
    }
#endif
}

void ImageViewer::copy() {
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(image);
#endif // !QT_NO_CLIPBOARD
}

#ifndef QT_NO_CLIPBOARD

static QImage clipboardImage() {
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull()) {
                return image;
            }
        }
    }
    return QImage();
}

#endif // !QT_NO_CLIPBOARD

void ImageViewer::paste() {
#ifndef QT_NO_CLIPBOARD
    const QImage newImage = clipboardImage();
    if (newImage.isNull()) {
        statusBar()->showMessage(tr("No image in clipboard"));
    } else {
        setImage(newImage);
        setWindowFilePath(QString());
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
                .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        statusBar()->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
}

void ImageViewer::zoomIn() {
    if (scaleFactor <= 10) {
        scaleImage(1.25);
    }
}

void ImageViewer::zoomOut() {
    if (scaleFactor > 0.1) {
        scaleImage(0.8);
    }
}

void ImageViewer::normalSize() {
    scaleFactor = 1.0;
    if (fitToWindowAct->isChecked()) {
        fitToWindow();
    } else {
        imageLabel->adjustSize();
    }
}

void ImageViewer::fitToWindow() {
    if (!image.isNull()) {
        bool fitToWindow = fitToWindowAct->isChecked();
        if (fitToWindow) {
            QSize size = image.size();
            const int delta = 2;
            double newScaleFactor = std::min((double) (scrollArea->width() - delta) / size.width(),
                                             (double) (scrollArea->height() - delta) / size.height());
            scalePixmap(std::min(scaleFactor, newScaleFactor));
        } else {
            scalePixmap(scaleFactor);
        }
    }
    updateActions();
}


void ImageViewer::about() {
    QMessageBox::about(this, tr("About Image Viewer"),
                       tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel "
                          "and QScrollArea to display an image. QLabel is typically used "
                          "for displaying a text, but it can also display an image. "
                          "QScrollArea provides a scrolling view around another widget. "
                          "If the child widget exceeds the size of the frame, QScrollArea "
                          "automatically provides scroll bars. </p><p>The example "
                          "demonstrates how QLabel's ability to scale its contents "
                          "(QLabel::scaledContents), and QScrollArea's ability to "
                          "automatically resize its contents "
                          "(QScrollArea::widgetResizable), can be used to implement "
                          "zooming and scaling features. </p><p>In addition the example "
                          "shows how to use QPainter to print an image.</p>"));
}

void ImageViewer::createActions() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &ImageViewer::open);
    openAct->setShortcut(QKeySequence::Open);

    saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &ImageViewer::saveAs);
    saveAsAct->setEnabled(false);

    printAct = fileMenu->addAction(tr("&Print..."), this, &ImageViewer::print);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setEnabled(false);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    copyAct = editMenu->addAction(tr("&Copy"), this, &ImageViewer::copy);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);

    QAction *pasteAct = editMenu->addAction(tr("&Paste"), this, &ImageViewer::paste);
    pasteAct->setShortcut(QKeySequence::Paste);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    auto *actionGroup = new QActionGroup(this);
    actionGroup->setExclusive(true);

    sortByNameAct = viewMenu->addAction(tr("Sort by name"), this, &ImageViewer::sortByName);
    sortByNameAct->setActionGroup(actionGroup);
    sortByNameAct->setCheckable(true);
    sortByNameAct->setChecked(true);

    sortByTimeAct = viewMenu->addAction(tr("Sort by time"), this, &ImageViewer::sortByTime);
    sortByTimeAct->setActionGroup(actionGroup);
    sortByTimeAct->setCheckable(true);

    viewMenu->addSeparator();
    sortReversedAct = viewMenu->addAction(tr("Reversed"), this, &ImageViewer::reverseSort);
    sortReversedAct->setCheckable(true);
    viewMenu->addSeparator();

    QAction *fullScreenAct = viewMenu->addAction(tr("Fullscreen mode"), this, &ImageViewer::fullScreenMode);
    fullScreenAct->setShortcut(QKeySequence::FullScreen);

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &ImageViewer::zoomIn);
    zoomInAct->setShortcut(tr("Ctrl+="));//QKeySequence::ZoomIn);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageViewer::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &ImageViewer::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+0"));

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &ImageViewer::fitToWindow);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    QMenu *settingsMenu = helpMenu->addMenu(tr("Settings"));

    settingsMenu->addAction(tr("&Restore Settings"), this, &ImageViewer::restoreSettings);
    settingsMenu->addAction(tr("&Save Geometry"), this, &ImageViewer::saveGeometry);
    settingsMenu->addAction(tr("&Save Sort"), this, &ImageViewer::saveSort);
    settingsMenu->addAction(tr("&Save Scale"), this, &ImageViewer::saveScale);
//    settings->addAction(tr("&Save Path"), this, &ImageViewer::savePath);
    settingsMenu->addAction(tr("&Drop Settings"), this, &ImageViewer::dropSettings);
    settingsMenu->addAction(tr("&Drop Geometry"), this, &ImageViewer::dropGeometry);
    settingsMenu->addAction(tr("&Drop Sort"), this, &ImageViewer::dropSort);
    settingsMenu->addAction(tr("&Drop Scale"), this, &ImageViewer::dropScale);
//    settings->addAction(tr("&Drop Path"), this, &ImageViewer::dropPath);

    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}

void ImageViewer::fullScreenMode() {
    if (isFullScreen()) {
        showNormal();
    } else if (isFullScreen()) {
        showFullScreen();
    }
}

void ImageViewer::sortByName() {
    iterator.setSortFlags(sortOrder().setFlag(QDir::Name));
}

void ImageViewer::sortByTime() {
    iterator.setSortFlags(sortOrder().setFlag(QDir::Time));
}

void ImageViewer::reverseSort() {
    QDir::SortFlags flags = iterator.getSortFlags();
    const QDir::SortFlags &order = sortOrder();
    iterator.setSortFlags(order.testFlag(QDir::Reversed) ? flags.setFlag(QDir::Reversed) : flags ^ QDir::Reversed);
}


void ImageViewer::updateActions() {
    saveAsAct->setEnabled(!image.isNull());
    copyAct->setEnabled(!image.isNull());
}

static void adjustScrollBar(QScrollBar *scrollBar, double factor);

void ImageViewer::scaleImage(double factor) {
    if (!image.isNull()) {
        scaleFactor *= factor;
        if (fitToWindowAct->isChecked()) {
            fitToWindow();
        } else {
            scalePixmap(scaleFactor);
        }
    }
}

void adjustScrollBar(QScrollBar *scrollBar, double factor) {
    int val = int(factor * scrollBar->value()
                  + ((factor - 1) * scrollBar->pageStep() / 2));
    scrollBar->setValue(val);
}

void ImageViewer::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Backspace:
            loadPrevious();
            break;
        case Qt::Key_Space:
            loadNext();
            break;
    }
}

void ImageViewer::resizeEvent(QResizeEvent *event) {
    if (fitToWindowAct->isChecked())
        fitToWindow();
}

void ImageViewer::loadNext() {
    if (iterator.canIterate()) {
        QFileInfo f(iterator.next());
        if (!f.isReadable()) {
            QFileInfo current(windowFilePath());
            do {
                f = iterator.next();
                if (f == current) {
                    return;
                }
            } while (!f.isReadable());
        }
        loadFile(f);
    }
}

void ImageViewer::loadPrevious() {
    if (iterator.canIterate()) {
        QFileInfo f(iterator.previous());
        if (!f.isReadable()) {
            QFileInfo current(windowFilePath());
            do {
                f = iterator.previous();
                if (f == current) {
                    return;
                }
            } while (!f.isReadable());
        }
        loadFile(f);
    }
}

QDir::SortFlags ImageViewer::sortOrder() {
    return sortReversedAct->isChecked() ? QDir::Reversed : QDir::Name;
}

void ImageViewer::scalePixmap(double factor) {
    imageLabel->resize(factor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);
}

void ImageViewer::loadImage(const QString &filePath) {
    loadFile(filePath);
}

void ImageViewer::restoreSettings() {
    restoreScale();
    restoreSort();
    restoreGeometry();
    restorePath();
}

void ImageViewer::restoreScale() {
    settings.beginGroup(SETTINGS_SCALE);
    scaleFactor = settings.value("scale", 1.0).value<double>();
    fitToWindowAct->setChecked(settings.value("fit", true).value<bool>());
    settings.endGroup();
}

void ImageViewer::restoreGeometry() {
    settings.beginGroup(SETTINGS_GEOMETRY);
    QSize wSize = settings.value("size", QSize()).value<QSize>();
    QPoint wPos = settings.value("pos", QPoint()).value<QPoint>();
    Qt::WindowStates wState = (Qt::WindowStates) settings.value("flags", (int) Qt::WindowActive).value<int>();
    settings.endGroup();


    resize(wSize.isEmpty() ? QGuiApplication::primaryScreen()->availableSize() * 3 / 5 : wSize);
    if (!wPos.isNull()) {
        move(wPos);
    }
    setWindowState(wState);
}

void ImageViewer::restoreSort() {
    QDir::SortFlags sortFlags = QDir::Name;
    if (settings.contains(SETTINGS_SORT)) {
        sortFlags = (QDir::SortFlags) settings.value(SETTINGS_SORT).value<int>();
    }
    iterator.setSortFlags(sortFlags);

    if (sortFlags.testFlag(QDir::Time)) {
        sortByTimeAct->setChecked(true);
    } else {
        sortByNameAct->setChecked(true);
    }

    sortReversedAct->setChecked(sortFlags.testFlag(QDir::Reversed));
}

//void ImageViewer::restorePath() {
//
//}

void ImageViewer::dropSettings() {
    settings.clear();
//    dropSort();
//    dropScale();
//    dropGeometry();
//    dropPath();
}

void ImageViewer::saveGeometry() {
    settings.beginGroup(SETTINGS_GEOMETRY);
    settings.setValue("flags", (int) windowState());
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}

void ImageViewer::saveSort() {
    settings.setValue(SETTINGS_SORT, (int) iterator.getSortFlags());
}

void ImageViewer::saveScale() {
    settings.beginGroup(SETTINGS_SCALE);
    settings.setValue("scale", scaleFactor);
    settings.setValue("fit", fitToWindowAct->isChecked());
    settings.endGroup();
}

//void ImageViewer::savePath() {
//
//}

void ImageViewer::dropGeometry() {
    settings.remove(SETTINGS_GEOMETRY);
}

void ImageViewer::dropSort() {
    settings.remove(SETTINGS_SORT);
}

void ImageViewer::dropScale() {
    settings.remove(SETTINGS_SCALE);
}

//void ImageViewer::dropPath() {
//
//}
