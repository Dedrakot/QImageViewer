//
// Created by ivan on 08.04.2021.
//

#ifndef IM_VIEW_IMAGEVIEWPORT_H
#define IM_VIEW_IMAGEVIEWPORT_H

#include <QWidget>

class ImageViewport {
protected:
    double scaleFactor = 1.0;
public:
    virtual ~ImageViewport() = default;

    [[nodiscard]] virtual QWidget *widget() const = 0;

    virtual void setImage(unsigned id, const QImage &image, bool fitToWindow) = 0;

    inline void scale(double scaleFactor) {
        setScaleFactor(getScaleFactor() * scaleFactor);
    }

    inline void setScaleFactor(double scaleFactor) {
        this->scaleFactor = scaleFactor;
    }

    inline double getScaleFactor() const {
        return scaleFactor;
    }

    virtual void zoomIn() = 0;

    virtual void zoomOut() = 0;

    virtual void normalSize() = 0;

    virtual void fitToWindow(bool value) = 0;

    virtual bool hasImage() = 0;

    virtual void copyToClipboard() = 0;

    virtual QImage getImage() const = 0;
};

#endif //IM_VIEW_IMAGEVIEWPORT_H
