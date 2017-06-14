#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "qcoapresource.h"


class tst_QCoapResource : public QObject
{
    Q_OBJECT

public:
    tst_QCoapResource();
    ~tst_QCoapResource();

private slots:
    void initTestCase();
    void cleanupTestCase();
};

tst_QCoapResource::tst_QCoapResource()
{
}

tst_QCoapResource::~tst_QCoapResource()
{
}

void tst_QCoapResource::initTestCase()
{
}

void tst_QCoapResource::cleanupTestCase()
{
}

QTEST_MAIN(tst_QCoapResource)

#include "tst_qcoapresource.moc"
