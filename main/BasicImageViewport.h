//
// Created by ivan on 08.04.2021.
//

#ifndef IM_VIEW_BASICIMAGEVIEWPORT_H
#define IM_VIEW_BASICIMAGEVIEWPORT_H

#include <QLabel>
#include <QScrollArea>
#include "ImageViewport.h"

class BasicImageViewport: public ImageViewport {
private:
    QLabel *imageLabel;
    QScrollArea *scrollArea;
public:
    explicit BasicImageViewport(QWidget *parent = nullptr);

    ~BasicImageViewport() override {
        delete imageLabel;
        delete scrollArea;
    }

    [[nodiscard]] QWidget* widget() const final {
        return scrollArea;
    }

    void setImage(const QImage &image) final;

    void scaleImage(double scaleFactor) final;

    [[nodiscard]] double calcFitToViewport(int statusBarGap) const final;
};


#endif //IM_VIEW_BASICIMAGEVIEWPORT_H
