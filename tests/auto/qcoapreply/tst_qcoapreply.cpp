#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
//#include "qcoapmessage.h"

//Q_DECLARE_METATYPE(QCoapMessage::QCoapMessageType)

class tst_QCoapReply : public QObject
{
    Q_OBJECT

public:
    tst_QCoapReply();
    ~tst_QCoapReply();

private slots:
    void initTestCase();
    void cleanupTestCase();

};

tst_QCoapReply::tst_QCoapReply()
{

}

tst_QCoapReply::~tst_QCoapReply()
{

}

void tst_QCoapReply::initTestCase()
{

}

void tst_QCoapReply::cleanupTestCase()
{

}

QTEST_MAIN(tst_QCoapReply)

#include "tst_qcoapreply.moc"
