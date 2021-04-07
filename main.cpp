#include <QApplication>
#include <QCommandLineParser>

#include "main/ImageViewer.h"
#include "main/OpenEventApp.h"

int main(int argc, char *argv[])
{
    OpenEventApp app(argc, argv);
    QGuiApplication::setApplicationDisplayName(ImageViewer::tr("Image Viewer"));
    QCommandLineParser commandLineParser;
    commandLineParser.addHelpOption();
    commandLineParser.addPositionalArgument(ImageViewer::tr("[file]"), ImageViewer::tr("Image file to open."));
    commandLineParser.process(QCoreApplication::arguments());
    ImageViewer imageViewer;
    if (!commandLineParser.positionalArguments().isEmpty()
        && !imageViewer.loadFile(QFileInfo(commandLineParser.positionalArguments().front()))) {
        return -1;
    }
    QObject::connect(&app, &OpenEventApp::openRequest, &imageViewer, &ImageViewer::loadImage);
    imageViewer.show();
    return QApplication::exec();
}
