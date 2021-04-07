//
// Created by ivan on 06.04.2021.
//

#include <QObject>
#include <QtTest/QtTest>

#include "utils.h"

#include <iostream>

class UtilsTest : public QObject {
Q_OBJECT
private slots:
    void test_ImgSupport() {
        auto filters = supportedImageFilters();
        std::cout << "Supported image filters: " << std::endl;
        for (const auto &filter: filters) {
            std::cout << filter.toUtf8().data() << std::endl;
        }
    }
};

QTEST_APPLESS_MAIN(UtilsTest)

#include "UtilsTest.moc"