//
// Created by ivan on 08.04.2021.
//

#ifndef IM_VIEW_IMAGEVIEWPORT_H
#define IM_VIEW_IMAGEVIEWPORT_H

#include <QWidget>

class ImageViewport {
public:
    virtual ~ImageViewport() = default;

    [[nodiscard]] virtual QWidget *widget() const = 0;

    virtual void setImage(const QImage &image) = 0;

    virtual void scaleImage(double scaleFactor) = 0;

    virtual double calcFitToViewport(int statusBarGap) const = 0;
};

#endif //IM_VIEW_IMAGEVIEWPORT_H
