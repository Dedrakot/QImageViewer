#include <QApplication>
#include <QCommandLineParser>

#include "main/AtomicIdChecker.h"
#include "main/ImageViewer.h"
#include "main/OpenEventApp.h"
#include "main/BasicImageViewport.h"

int main(int argc, char *argv[]) {
    OpenEventApp app(argc, argv);
    QGuiApplication::setApplicationDisplayName(ImageViewer::tr("Image Viewer"));
    QCommandLineParser commandLineParser;
    commandLineParser.addHelpOption();
    commandLineParser.addPositionalArgument(ImageViewer::tr("[file]"), ImageViewer::tr("Image file to open."));
    commandLineParser.process(QCoreApplication::arguments());
    AtomicIdChecker idChecker;
    BasicImageViewport imageViewport(idChecker);
    ImageViewer imageViewer(imageViewport, idChecker);
#ifdef Q_OS_MAC
    imageViewer.connect(&imageViewport, &BasicImageViewport::repaintRequired, &imageViewer, &ImageViewer::repaintForMac);
#endif
    if (!commandLineParser.positionalArguments().isEmpty()) {
        imageViewer.loadFile(QFileInfo(commandLineParser.positionalArguments().front()));
    }
    QObject::connect(&app, &OpenEventApp::openRequest, &imageViewer, &ImageViewer::loadImage);
    imageViewer.show();
    return QApplication::exec();
}
