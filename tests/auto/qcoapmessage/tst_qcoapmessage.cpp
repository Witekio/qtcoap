#include <QtTest>
#include <QCoreApplication>

// add necessary includes here

class QCoapMessage : public QObject
{
    Q_OBJECT

public:
    QCoapMessage();
    ~QCoapMessage();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

};

QCoapMessage::QCoapMessage()
{

}

QCoapMessage::~QCoapMessage()
{

}

void QCoapMessage::initTestCase()
{

}

void QCoapMessage::cleanupTestCase()
{

}

void QCoapMessage::test_case1()
{

}

QTEST_MAIN(QCoapMessage)

#include "tst_qcoapmessage.moc"
