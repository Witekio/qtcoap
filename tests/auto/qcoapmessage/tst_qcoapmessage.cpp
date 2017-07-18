#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "qcoapmessage.h"

Q_DECLARE_METATYPE(QCoapMessage::QCoapMessageType)

class tst_QCoapMessage : public QObject
{
    Q_OBJECT

public:
    tst_QCoapMessage();
    ~tst_QCoapMessage();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void setMessageType_data();
    void setMessageType();
};

tst_QCoapMessage::tst_QCoapMessage()
{
}

tst_QCoapMessage::~tst_QCoapMessage()
{
}

void tst_QCoapMessage::initTestCase()
{
}

void tst_QCoapMessage::cleanupTestCase()
{
}

void tst_QCoapMessage::setMessageType_data()
{
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");

    QTest::newRow("acknowledgment") << QCoapMessage::AcknowledgmentCoapMessage;
    QTest::newRow("confirmable") << QCoapMessage::ConfirmableCoapMessage;
    QTest::newRow("non-confirmable") << QCoapMessage::NonConfirmableCoapMessage;
    QTest::newRow("reset") << QCoapMessage::ResetCoapMessage;
}

void tst_QCoapMessage::setMessageType()
{
    QFETCH(QCoapMessage::QCoapMessageType, type);
    QCoapMessage message;
    message.setType(type);
    QCOMPARE(message.type(), type);
}

QTEST_MAIN(tst_QCoapMessage)

#include "tst_qcoapmessage.moc"
