#include <QObject>
#include <QtTest/QtTest>

class MyFirstTest: public QObject {
    Q_OBJECT
private:
    bool myCondition()
    {
        return true;
    }

private slots:
    void edu_FileInfoDir() {
        QFileInfo f;
        QDir d(f.dir());

        QCOMPARE(d.path(), QString("."));
    }

    void initTestCase()
    {
        qDebug("Called before everything else.");
    }

    void myFirstTest()
    {
        QVERIFY(true); // check that a condition is satisfied
        QCOMPARE(1, 1); // compare two values
    }

    void mySecondTest()
    {
        QVERIFY(myCondition());
        QVERIFY(1 != 2);
    }

    void cleanupTestCase()
    {
        qDebug("Called after myFirstTest and mySecondTest.");
    }
};

QTEST_APPLESS_MAIN(MyFirstTest)

#include "test.moc"
