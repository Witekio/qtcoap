#include <QtTest>
#include <QCoreApplication>

#include <QtCoap/qcoapmessage.h>

class tst_QCoapMessage : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void copyAndDetach();
    void setMessageType_data();
    void setMessageType();
    void removeOption();
};

void tst_QCoapMessage::copyAndDetach()
{
    QCoapMessage a;
    a.setMessageId(3);
    a.setPayload("payload");
    a.setToken("token");
    a.setType(QCoapMessage::Acknowledgement);
    a.setVersion(5);

    // Test the copy
    QCoapMessage b(a);
    QVERIFY2(b.messageId() == 3, "Message not copied correctly");
    QVERIFY2(b.payload() == "payload", "Message not copied correctly");
    QVERIFY2(b.token() == "token", "Message not copied correctly");
    QVERIFY2(b.type() == QCoapMessage::Acknowledgement, "Message not copied correctly");
    QVERIFY2(b.version() == 5, "Message not copied correctly");

    // Detach
    b.setMessageId(9);
    QCOMPARE(b.messageId(), 9);
    QCOMPARE(a.messageId(), 3);
}

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

    //! TODO extend QCoapMessage tests
}

void tst_QCoapMessage::removeOption()
{
    //! TODO with one and more than one identical options
}

QTEST_APPLESS_MAIN(tst_QCoapMessage)

#include "tst_qcoapmessage.moc"
