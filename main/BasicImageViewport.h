//
// Created by ivan on 08.04.2021.
//

#ifndef IM_VIEW_BASICIMAGEVIEWPORT_H
#define IM_VIEW_BASICIMAGEVIEWPORT_H

#include <QLabel>
#include <QScrollArea>
#include "IdChecker.h"
#include "ImageViewport.h"

class BasicImageViewport: public QObject, public ImageViewport {
    Q_OBJECT
private:
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    const IdChecker<unsigned> &idChecker;
public:
    explicit BasicImageViewport(const IdChecker<unsigned> &idChecker, QWidget *parent = nullptr);

    ~BasicImageViewport() override {
        delete imageLabel;
        delete scrollArea;
    }

    [[nodiscard]] QWidget* widget() const final {
        return scrollArea;
    }

    void setImage(unsigned id, const QImage &image, bool fitToWindow) final;

    void zoomIn() override;

    void zoomOut() override;

    void normalSize() override;

    void fitToWindow(bool value) override;

    bool hasImage() override;

    void copyToClipboard() override;

    QImage getImage() const override;

signals:
    void newLabelPixmap(unsigned id, const QPixmap &pixmap, bool fitToWindow);
#ifdef Q_OS_MAC
    void repaintRequired();
#endif
private slots:
    void setLabelPixmap(unsigned id, const QPixmap &pixmap, bool fitToWindow);
    void firstImageInit(unsigned id, const QPixmap &pixmap, bool fitToWindow);

private:
    void scaleImage();

    [[nodiscard]] double calcFitToViewport() const;

    bool canZoom(double factor);

    void changeScale(double factor);
};


#endif //IM_VIEW_BASICIMAGEVIEWPORT_H
