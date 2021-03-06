//
// Created by ivan on 06.04.2021.
//
#include <qglobal.h>

#include "ImageViewer.h"

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
#include <QStandardPaths>
#include <QStatusBar>
#include <QKeyEvent>
#include <QtCore/QSettings>
#include <QActionGroup>
#include <QtConcurrent/QtConcurrent>

#include "utils.h"
#include "AtomicIdChecker.h"

#if defined(QT_PRINTSUPPORT_LIB)
#  include <QtPrintSupport/qtprintsupportglobal.h>

#  if QT_CONFIG(printdialog)
#    include <QPrintDialog>
#  endif
#endif

const QString SETTINGS_SORT = "view/sort";
const QString SETTINGS_GEOMETRY = "view/geometry";
const QString SETTINGS_SCALE = "view/scale";
const QString SETTINGS_PATH = "view/path";

ImageViewer::ImageViewer(ImageViewport &viewport, IdChecker<unsigned> &idChecker, QWidget *parent) :
        QMainWindow(parent),
        imageViewPort(viewport),
        idChecker(idChecker),
        settings("Dedrakot", "ImageViewer"),
        iterator(supportedImageFilters()) {

    statusBar()->showMessage(tr("No data loaded"));
//    statusBar()->setVisible(true);
    connect(this, &ImageViewer::newStatus, this, &ImageViewer::showStatus);
    setCentralWidget(imageViewPort.widget());

    createActions();

    restoreSettings();
}

void ImageViewer::loadFileAsync(const QString &filePath, unsigned loadId) {
    if (idChecker.isCanceled(loadId))
        return;
    QImageReader reader(filePath);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    if (image.isNull()) {
        const QString message = tr("Cannot load %1: %2")
                .arg(QDir::toNativeSeparators(filePath), reader.errorString());
        emit newStatus(message);
    } else {
        setImage(image, loadId);
    }
}

void ImageViewer::loadFile(const QFileInfo &file) {
    iterator.setFile(file);
    QString filePath = file.filePath();
    unsigned nextLoadId = idChecker.nextId();
    loadFuture.cancel();
    loadFuture = QtConcurrent::run(&ImageViewer::loadFileAsync, this, filePath, nextLoadId);
    setWindowFilePath(filePath);
}

void ImageViewer::setImage(QImage &image, unsigned int loadId) {
    if (idChecker.isCanceled(loadId))
        return;
    if (image.isNull()) {
        const QString message = tr("Invalid image");
        emit newStatus(message);
        return;
    }

    if (image.colorSpace().isValid())
        image.convertToColorSpace(QColorSpace::SRgb);

    imageViewPort.setImage(loadId, image, fitToWindowAct->isChecked());
}

bool ImageViewer::saveFile(const QString &fileName) {
    QImageWriter writer(fileName);

    if (!writer.write(imageViewPort.getImage())) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                         .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}

QString currentPath(const QString &current) {
    if (!current.isEmpty() && QDir(current).exists()) {
        return current;
    }
    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    return picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last();
}

static bool firstDialog = true;

static void
initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode, const QSettings &settings) {
    if (firstDialog) {
        firstDialog = false;
        dialog.setDirectory(currentPath(settings.value(SETTINGS_PATH, QString()).value<QString>()));
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
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen, settings);

    if (dialog.exec() == QDialog::Accepted)
        loadFile(QFileInfo(dialog.selectedFiles().first()));
}

void ImageViewer::saveAs() {
    if (!imageViewPort.hasImage())
        return;
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave, settings);

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}

void ImageViewer::deleteFile() {
    QFile file(iterator.current().absoluteFilePath());

    if (QMessageBox::Yes == QMessageBox::question(this, tr("Remove file"),
                                                  QString(tr("Do you want to remove \"%1\"")).arg(file.fileName()))) {
        file.remove();
        iterator.remove();
        loadFile(iterator.current());
    }
}

void ImageViewer::print() {
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printdialog)
    if (!imageViewPort->hasImage());
        return;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = image.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(image.rect());
        painter.drawImage(0, 0, image);
    }
#endif
}

void ImageViewer::copy() {
    imageViewPort.copyToClipboard();
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
    QImage newImage = clipboardImage();
    if (newImage.isNull()) {
        statusBar()->showMessage(tr("No image in clipboard"));
    } else {
        setImage(newImage, idChecker.nextId());
        setWindowFilePath(QString());
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
                .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        statusBar()->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
}

void ImageViewer::zoomIn() {
    imageViewPort.zoomIn();
}

void ImageViewer::zoomOut() {
    imageViewPort.zoomOut();
}

void ImageViewer::normalSize() {
    imageViewPort.normalSize();
}

void ImageViewer::fitToWindow() {
    imageViewPort.fitToWindow(fitToWindowAct->isChecked());
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
    saveAsAct->setShortcut(QKeySequence::SaveAs);

#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    printAct = fileMenu->addAction(tr("&Print..."), this, &ImageViewer::print);
    printAct->setShortcut(QKeySequence::Print);
#endif

    auto deleteAct = fileMenu->addAction(tr("&Delete"), this, &ImageViewer::deleteFile);
    deleteAct->setShortcut(QKeySequence::Delete);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    copyAct = editMenu->addAction(tr("&Copy"), this, &ImageViewer::copy);
    copyAct->setShortcut(QKeySequence::Copy);

    QAction *pasteAct = editMenu->addAction(tr("&Paste"), this, &ImageViewer::paste);
    pasteAct->setShortcut(QKeySequence::Paste);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    auto *actionGroup = new QActionGroup(this);
    actionGroup->setExclusive(true);

    sortByNameAct = viewMenu->addAction(tr("Sort by name"), this, &ImageViewer::sortByName);
    sortByNameAct->setActionGroup(actionGroup);
    sortByNameAct->setCheckable(true);
    sortByNameAct->setChecked(true);
    sortByNameAct->setData(QDir::Name);

    sortByTimeAct = viewMenu->addAction(tr("Sort by time"), this, &ImageViewer::sortByTime);
    sortByTimeAct->setActionGroup(actionGroup);
    sortByTimeAct->setCheckable(true);
    sortByTimeAct->setData(QDir::Time);


    viewMenu->addSeparator();
    sortReversedAct = viewMenu->addAction(tr("Reversed"), this, &ImageViewer::reverseSort);
    sortReversedAct->setCheckable(true);
    viewMenu->addSeparator();

    QAction *fullScreenAct = viewMenu->addAction(tr("Fullscreen mode"), this, &ImageViewer::fullScreenMode);
    fullScreenAct->setShortcut(QKeySequence::FullScreen);

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &ImageViewer::zoomIn);
    zoomInAct->setShortcuts({tr("Ctrl+="), tr("Ctrl++")});//, QKeySequence::ZoomIn});

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageViewer::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &ImageViewer::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+0"));

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &ImageViewer::fitToWindow);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    QMenu *settingsMenu = menuBar()->addMenu(tr("Settings"));

    settingsMenu->addAction(tr("&Restore Settings"), this, &ImageViewer::restoreSettings);
    settingsMenu->addAction(tr("&Save Geometry"), this, &ImageViewer::saveGeometry);
    settingsMenu->addAction(tr("&Save Sort"), this, &ImageViewer::saveSort);
    settingsMenu->addAction(tr("&Save Scale"), this, &ImageViewer::saveScale);
    settingsMenu->addAction(tr("&Save Path"), this, &ImageViewer::savePath);
    settingsMenu->addAction(tr("&Drop Settings"), this, &ImageViewer::dropSettings);
    settingsMenu->addAction(tr("&Drop Geometry"), this, &ImageViewer::dropGeometry);
    settingsMenu->addAction(tr("&Drop Sort"), this, &ImageViewer::dropSort);
    settingsMenu->addAction(tr("&Drop Scale"), this, &ImageViewer::dropScale);
    settingsMenu->addAction(tr("&Drop Path"), this, &ImageViewer::dropPath);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}

void ImageViewer::fullScreenMode() {
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void ImageViewer::sortByName() {
    iterator.setSortFlags(sortFlags().setFlag(QDir::Name));
}

void ImageViewer::sortByTime() {
    iterator.setSortFlags(sortFlags().setFlag(QDir::Time));
}

void ImageViewer::reverseSort() {
    iterator.setSortFlags(sortFlags());
}

void ImageViewer::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Backspace:
            initPrevious();
            break;
        case Qt::Key_Space:
            initNext();
            break;
        default:
            QMainWindow::keyPressEvent(event);
    }
}

void ImageViewer::keyReleaseEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Backspace:
        case Qt::Key_Space:
            load();
            break;
        default:
            QWidget::keyReleaseEvent(event);
    }
}

void ImageViewer::resizeEvent(QResizeEvent *event) {
    if (fitToWindowAct->isChecked())
        fitToWindow();
}

QFileInfo iterate(FileIterator &iterator, QFileInfo (FileIterator::*next)()) {
    QFileInfo f((&iterator->*next)());
    if (!f.fileName().isEmpty() && !f.isReadable()) {
        QFileInfo current(f);
        do {
            iterator.remove();
            f = (&iterator->*next)();
        } while (!f.isReadable() && !f.fileName().isEmpty());
    }
    return f;
}

void ImageViewer::initNext() {
    QFileInfo f(iterate(iterator, &FileIterator::next));
    bool notifyLoop = false;
    if (f.fileName().isEmpty()) {
        f = iterator.first();
        notifyLoop = true;
    }
    if (!f.isReadable() && !iterator.isEmpty()) {
        f = iterator.previous();
        iterator.reloadDir();
    }
    showNextFileStatus(f, notifyLoop ? tr("Returned to the first one") : QString());
}

void ImageViewer::initPrevious() {
    QFileInfo f(iterate(iterator, &FileIterator::previous));
    bool notifyLoop = false;
    if (f.fileName().isEmpty()) {
        f = iterator.last();
        notifyLoop = true;
    }

    while (!f.isReadable() && !iterator.isEmpty()) {
        f = iterator.next();
        iterator.reloadDir();
    }

    showNextFileStatus(f, notifyLoop ? tr("Returned to the last one") : QString());
}

void ImageViewer::load() {
    QFileInfo f = iterator.current();
    if (f.isReadable() && windowFilePath() != f.filePath()) {
        loadFile(f);
    }
}

QDir::SortFlags ImageViewer::sortFlags() {
    QDir::SortFlags flags = QDir::Name;
    QDir::SortFlags reverse = sortReversedAct->isChecked() ? QDir::Reversed : QDir::Name;
    for (const auto &sortAction: sortByNameAct->actionGroup()->actions()) {
        if (sortAction->isChecked()) {
            return static_cast<QDir::SortFlags>(sortAction->data().toInt()) ^ reverse;

        }
    }
    return flags ^ reverse;
}

void ImageViewer::loadImage(const QString &filePath) {
    loadFile(QFileInfo(filePath));
}

void ImageViewer::restoreSettings() {
    restoreScale();
    restoreSort();
    restoreGeometry();
    restorePath();
}

void ImageViewer::restoreScale() {
    settings.beginGroup(SETTINGS_SCALE);
    imageViewPort.setScaleFactor(settings.value("scale", 1.0).value<double>());
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

void ImageViewer::restorePath() {
    firstDialog = true;
}

void ImageViewer::dropSettings() {
    settings.clear();
}

void ImageViewer::saveGeometry() {
    settings.beginGroup(SETTINGS_GEOMETRY);
    settings.setValue("flags", (int) windowState());
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}

void ImageViewer::saveSort() {
    settings.setValue(SETTINGS_SORT, (int) sortFlags());
}

void ImageViewer::saveScale() {
    settings.beginGroup(SETTINGS_SCALE);
    settings.setValue("scale", imageViewPort.getScaleFactor());
    settings.setValue("fit", fitToWindowAct->isChecked());
    settings.endGroup();
}

void ImageViewer::savePath() {
    QFileInfo filePath(windowFilePath());
    settings.setValue(SETTINGS_PATH, (filePath.isDir() ? QDir(filePath.path()) : filePath.dir()).absolutePath());
}

void ImageViewer::dropGeometry() {
    settings.remove(SETTINGS_GEOMETRY);
}

void ImageViewer::dropSort() {
    settings.remove(SETTINGS_SORT);
}

void ImageViewer::dropScale() {
    settings.remove(SETTINGS_SCALE);
}

void ImageViewer::dropPath() {
    settings.remove(SETTINGS_PATH);
}

#ifdef Q_OS_MAC
bool ImageViewer::event(QEvent *event) {
    if (event->type() == QEvent::NativeGesture) {
        QNativeGestureEvent *nge = static_cast<QNativeGestureEvent *>(event);
        if (nge->gestureType() == Qt::ZoomNativeGesture) {
            double v = nge->value();
            double factor;
            if (std::abs(v)>0.002) {
                factor = 1.0 + v;
                if (!canZoom(factor)) {
                    return true;
                }
                imageViewPort.scale(factor);
                const QString message = tr("Zoom diff %1. Scale: %2")
                        .arg(factor).arg(scaleFactor);
                statusBar()->showMessage(message);
            }
            return true;
        }
    }
    return QMainWindow::event(event);
}
#endif

void ImageViewer::showStatus(const QString &message) {
    statusBar()->showMessage(message);
}

void ImageViewer::showNextFileStatus(const QFileInfo &f, const QString &addition) {
    if (f.isReadable() && windowFilePath() != f.filePath()) {
        QString message = tr("(%1/%2) \"%3\"")
                .arg(iterator.currentPosition())
                .arg(iterator.directorySize())
                .arg(f.fileName());
        if (!addition.isEmpty()) {
            message += " | " + addition;
        }
        statusBar()->showMessage(message);
    }
}

#ifdef Q_OS_MAC
void ImageViewer::repaintForMac() {
    const Qt::WindowStates &wState = windowState();
    if (wState.testFlag(Qt::WindowMaximized) || wState.testFlag(Qt::WindowFullScreen)) {
        repaint();
    }
}
#endif