//
// Created by ivan on 06.04.2021.
//

#include <QObject>
#include <QtTest/QtTest>

#include "../main/AsyncFileIterator.h"

const QString TEST_DATA_DIR = "TestData";

bool deleteChildFiles(const QString &dirName);

void createFiles(int first, int last);

void createFilesReverseModifiedTime(int first, int last);

QFileInfo file(int n);


class AsyncFiletIteratorTest : public QObject {
Q_OBJECT
private slots:

    void initTestCase() {
        QDir dir;
        dir.mkdir(TEST_DATA_DIR);
    }

    void test_empty() {
        AsyncFileIterator filetIterator;

        QCOMPARE(filetIterator.current().fileName(), QString());
        QCOMPARE(filetIterator.next().fileName(), QString());
        QCOMPARE(filetIterator.previous().fileName(), QString());
        QCOMPARE(filetIterator.directorySize(), 0);
    }

    void test_reload() {
        AsyncFileIterator filetIterator;
        createFiles(1, 2);
        createFiles(4, 5);

        filetIterator.setFile(QFileInfo(file(4)));
        QCOMPARE(filetIterator.previous().fileName(), file(2).fileName());

        createFiles(3, 3);

        QCOMPARE(filetIterator.next().fileName(), file(4).fileName());
        QCOMPARE(filetIterator.directorySize(), 4);
        filetIterator.reloadDir();
        QCOMPARE(filetIterator.previous().fileName(), file(3).fileName());
        QCOMPARE(filetIterator.directorySize(), 5);

        QCOMPARE(dirSize(filetIterator), 5);
    }

    void test_orderByName() {
        AsyncFileIterator filetIterator;
        createFiles(15, 20);

        filetIterator.setSortFlags(QDir::Name);
        filetIterator.setFile(QFileInfo(file(16)));
        for (int i = 17; i <= 20; i++) {
            QCOMPARE(filetIterator.next().fileName(), file(i).fileName());
        }

        filetIterator.setSortFlags(QDir::Name | QDir::Reversed);
        for (int i = 19; i >= 15; i--) {
            QCOMPARE(filetIterator.next().fileName(), file(i).fileName());
        }
    }

    void test_orderByTime() {
        AsyncFileIterator filetIterator;
        createFilesReverseModifiedTime(15, 20);

        filetIterator.setSortFlags(QDir::Time | QDir::Reversed);
        filetIterator.setFile(QFileInfo(file(19)));
        for (int i = 18; i >= 15; i--) {
            QCOMPARE(filetIterator.next().fileName(), file(i).fileName());
        }

        filetIterator.setSortFlags(QDir::Time);
        for (int i = 16; i <= 20; i++) {
            QCOMPARE(filetIterator.next().fileName(), file(i).fileName());
        }
    }

    void test_orderByTimeWithSameTime() {
        AsyncFileIterator filetIterator;
        createFiles(15, 20);

        filetIterator.setSortFlags(QDir::Time | QDir::Reversed);
        filetIterator.setFile(QFileInfo(file(19)));
        for (int i = 18; i >= 15; i--) {
            QCOMPARE(filetIterator.next().fileName(), file(i).fileName());
        }

        filetIterator.setSortFlags(QDir::Time);
        for (int i = 16; i <= 20; i++) {
            QCOMPARE(filetIterator.next().fileName(), file(i).fileName());
        }
    }

    void cleanup() {
        deleteChildFiles(TEST_DATA_DIR);
    }

    void cleanupTestCase() {
        QDir dir(TEST_DATA_DIR);
        if (!dir.isEmpty()) {
            QFAIL("TestData directory is not empty after tests");
        }
    }

private:
    int dirSize(FileIterator &fi) {
        return fi.directorySize();
    }
};

QTEST_APPLESS_MAIN(AsyncFiletIteratorTest)

#include "AsyncFileIteratorTest.moc"

bool deleteChildFiles(const QString &dirName) {
    bool result = true;
    QDir dir(dirName);

    if (dir.exists()) {
        Q_FOREACH(QFileInfo info,
                  dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files,
                                    QDir::DirsFirst)) {
                if (info.isDir()) {
                    result = result && info.dir().removeRecursively();
                } else {
                    result = result && QFile::remove(info.absoluteFilePath());
                }
            }
    }
    return result;
}

void createFiles(int first, int last) {
    QDir testDir(TEST_DATA_DIR);
    for (int i = first; i <= last; i++) {
        QFile f(testDir.filePath(QString::number(i)));
        if (f.open(QFile::NewOnly | QFile::WriteOnly)) {
            f.write("\0");
            f.close();
        }
    }
}

void createFilesReverseModifiedTime(int first, int last) {
    QDir testDir(TEST_DATA_DIR);
    for (int i = first; i <= last; i++) {
        QFile f(testDir.filePath(QString::number(i)));
        if (f.open(QFile::NewOnly | QFile::WriteOnly)) {
            f.write("\0");
            // reverse to name order
            f.setFileTime(f.fileTime(QFileDevice::FileModificationTime).addSecs(-i), QFileDevice::FileModificationTime);
            f.close();
        }
    }
}

QFileInfo file(int n) {
    return QFileInfo(QDir(TEST_DATA_DIR), QString::number(n));
}