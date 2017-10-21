#include <QtTest>
#include <QCoreApplication>

#include <QtCoap/qcoapmessage.h>

class tst_QCoapMessage : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void setMessageType_data();
    void setMessageType();
};

void tst_QCoapMessage::setMessageType_data()
{
    QTest::addColumn<QCoapMessage::MessageType>("type");

    QTest::newRow("acknowledgement") << QCoapMessage::Acknowledgement;
    QTest::newRow("confirmable") << QCoapMessage::Confirmable;
    QTest::newRow("non-confirmable") << QCoapMessage::NonConfirmable;
    QTest::newRow("reset") << QCoapMessage::Reset;
}

void tst_QCoapMessage::setMessageType()
{
    QFETCH(QCoapMessage::MessageType, type);
    QCoapMessage message;
    message.setType(type);
    QCOMPARE(message.type(), type);
}

QTEST_MAIN(tst_QCoapMessage)

#include "tst_qcoapmessage.moc"
