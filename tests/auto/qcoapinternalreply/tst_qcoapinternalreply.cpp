/****************************************************************************
**
** Copyright (C) 2018 Witekio.
** Contact: https://witekio.com/contact/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest>
#include <QCoreApplication>

#include <private/qcoapinternalreply_p.h>
#include <private/qcoapreply_p.h>

class tst_QCoapInternalReply : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void parseReplyPdu_data();
    void parseReplyPdu();
    void updateReply_data();
    void updateReply();
    void requestData();
    void abortRequest();
};

void tst_QCoapInternalReply::parseReplyPdu_data()
{
    QTest::addColumn<QtCoap::ResponseCode>("responseCode");
    QTest::addColumn<QCoapMessage::MessageType>("type");
    QTest::addColumn<quint16>("messageId");
    QTest::addColumn<QByteArray>("token");
    QTest::addColumn<quint8>("tokenLength");
    QTest::addColumn<QList<QCoapOption::OptionName>>("optionsNames");
    QTest::addColumn<QList<quint8>>("optionsLengths");
    QTest::addColumn<QList<QByteArray>>("optionsValues");
    QTest::addColumn<QString>("payload");
    QTest::addColumn<QString>("pduHexa");

    QList<QCoapOption::OptionName> optionsNamesReply({QCoapOption::ContentFormat,
                                                      QCoapOption::MaxAge});
    QList<quint8> optionsLengthsReply({0, 1});
    QList<QByteArray> optionsValuesReply({"", QByteArray::fromHex("1e")});

    QList<QCoapOption::OptionName> bigOptionNameReply({QCoapOption::Size1});
    QList<quint8> bigOptionLengthReply({26});
    QList<QByteArray> bigOptionValueReply({QByteArray("abcdefghijklmnopqrstuvwxyz")});

    QTest::newRow("reply_with_options_and_payload")
            << QtCoap::Content
            << QCoapMessage::NonConfirmable
            << quint16(64463)
            << QByteArray("4647f09b")
            << quint8(4)
            << optionsNamesReply
            << optionsLengthsReply
            << optionsValuesReply
            << "Type: 1 (NON)\nCode: 1 (GET)\nMID: 56400\nToken: 4647f09b"
            << "5445fbcf4647f09bc0211eff547970653a203120284e4f4e290a436f64653a20"
               "312028474554290a4d49443a2035363430300a546f6b656e3a20343634376630"
               "3962";

    QTest::newRow("reply_with_payload")
            << QtCoap::Content
            << QCoapMessage::NonConfirmable
            << quint16(64463)
            << QByteArray("4647f09b")
            << quint8(4)
            << QList<QCoapOption::OptionName>()
            << QList<quint8>()
            << QList<QByteArray>()
            << "Type: 1 (NON)\nCode: 1 (GET)\nMID: 56400\nToken: 4647f09b"
            << "5445fbcf4647f09bff547970653a203120284e4f4e290a436f64653a20312028"
               "474554290a4d49443a2035363430300a546f6b656e3a203436343766303962";

    QTest::newRow("reply_with_options")
            << QtCoap::Content
            << QCoapMessage::NonConfirmable
            << quint16(64463)
            << QByteArray("4647f09b")
            << quint8(4)
            << optionsNamesReply
            << optionsLengthsReply
            << optionsValuesReply
            << ""
            << "5445fbcf4647f09bc0211e";

    QTest::newRow("reply_only")
            << QtCoap::Content
            << QCoapMessage::NonConfirmable
            << quint16(64463)
            << QByteArray("4647f09b")
            << quint8(4)
            << QList<QCoapOption::OptionName>()
            << QList<quint8>()
            << QList<QByteArray>()
            << ""
            << "5445fbcf4647f09b";

    QTest::newRow("reply_with_big_option")
            << QtCoap::Content
            << QCoapMessage::NonConfirmable
            << quint16(64463)
            << QByteArray("4647f09b")
            << quint8(4)
            << bigOptionNameReply
            << bigOptionLengthReply
            << bigOptionValueReply
            << ""
            << "5445fbcf4647f09bdd2f0d6162636465666768696a6b6c6d6e6f707172737475"
               "767778797a";
}

void tst_QCoapInternalReply::parseReplyPdu()
{
    QFETCH(QtCoap::ResponseCode, responseCode);
    QFETCH(QCoapMessage::MessageType, type);
    QFETCH(quint16, messageId);
    QFETCH(QByteArray, token);
    QFETCH(quint8, tokenLength);
    QFETCH(QList<QCoapOption::OptionName>, optionsNames);
    QFETCH(QList<quint8>, optionsLengths);
    QFETCH(QList<QByteArray>, optionsValues);
    QFETCH(QString, payload);
    QFETCH(QString, pduHexa);

    QScopedPointer<QCoapInternalReply>
            reply(QCoapInternalReply::createFromFrame(QByteArray::fromHex(pduHexa.toUtf8())));

    QCOMPARE(reply->message()->type(), type);
    QCOMPARE(reply->message()->tokenLength(), tokenLength);
    QCOMPARE(reply->responseCode(), responseCode);
    QCOMPARE(reply->message()->messageId(), messageId);
    QCOMPARE(reply->message()->token().toHex(), token);
    QCOMPARE(reply->message()->optionCount(), optionsNames.count());
    for (int i = 0; i < reply->message()->optionCount(); ++i) {
        QCoapOption option = reply->message()->option(i);
        QCOMPARE(option.name(), optionsNames.at(i));
        QCOMPARE(option.length(), optionsLengths.at(i));
        QCOMPARE(option.value(), optionsValues.at(i));
    }
    QCOMPARE(reply->message()->payload(), payload);
}

class QCoapReplyForTests : public QCoapReply
{
public:
    QCoapReplyForTests(const QCoapRequest &req) : QCoapReply (req) {}

    void setRunning(const QCoapToken &token, QCoapMessageId messageId)
    {
        Q_D(QCoapReply);
        d->_q_setRunning(token, messageId);
    }
    void setContentAndFinished(const QCoapInternalReply *internal)
    {
        Q_D(QCoapReply);
        d->_q_setContent(internal->senderAddress(), *internal->message(), internal->responseCode());
        d->_q_setFinished();
    }
};

void tst_QCoapInternalReply::updateReply_data()
{
    QTest::addColumn<QByteArray>("data");

    QTest::newRow("success") << QByteArray("Data for the updating test");
}

void tst_QCoapInternalReply::updateReply()
{
    QFETCH(QByteArray, data);

    QCoapReplyForTests reply((QCoapRequest()));
    QCoapInternalReply internalReply;
    internalReply.message()->setPayload(data);
    QSignalSpy spyReplyFinished(&reply, &QCoapReply::finished);

    reply.setContentAndFinished(&internalReply);

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 1000);
    QCOMPARE(reply.readAll(), data);
}

void tst_QCoapInternalReply::requestData()
{
    QCoapReplyForTests reply((QCoapRequest()));
    reply.setRunning("token", 543);

    QCOMPARE(reply.request().token(), QByteArray("token"));
    QCOMPARE(reply.request().messageId(), 543);
}

void tst_QCoapInternalReply::abortRequest()
{
    QCoapReplyForTests reply((QCoapRequest()));
    reply.setRunning("token", 543);

    QSignalSpy spyAborted(&reply, &QCoapReply::aborted);
    QSignalSpy spyFinished(&reply, &QCoapReply::finished);
    reply.abortRequest();

    QTRY_COMPARE_WITH_TIMEOUT(spyAborted.count(), 1, 1000);
    QList<QVariant> arguments = spyAborted.takeFirst();
    QTRY_COMPARE_WITH_TIMEOUT(spyFinished.count(), 1, 1000);
    QVERIFY(arguments.at(0).toByteArray() == "token");
}

QTEST_MAIN(tst_QCoapInternalReply)

#include "tst_qcoapinternalreply.moc"
