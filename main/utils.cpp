//
// Created by ivan on 06.04.2021.
//

#include "utils.h"

#include <QtCore/QMimeDatabase>
#include <QtGui/QImageReader>

QStringList supportedImageFilters()
{
    QStringList ret;
    QMimeDatabase db;
    for (const QByteArray &mime: QImageReader::supportedMimeTypes()) {
        const QMimeType &t = db.mimeTypeForName(mime);
        ret.append(t.globPatterns());
    }
    return ret;
}
