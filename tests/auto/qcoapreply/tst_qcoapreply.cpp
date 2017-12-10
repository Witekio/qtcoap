/****************************************************************************
**
** Copyright (C) 2017 Witekio.
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

#include <QtCoap/qcoapreply.h>
#include <private/qcoapinternalreply_p.h>

class tst_QCoapReply : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void parseReplyPdu_data();
    void parseReplyPdu();
    void updateReply_data();
    void updateReply();
};

void tst_QCoapReply::parseReplyPdu_data()
{
    QTest::addColumn<QtCoap::StatusCode>("statusCode");
    QTest::addColumn<QCoapMessage::MessageType>("type");
    QTest::addColumn<quint16>("messageId");
    QTest::addColumn<QByteArray>("token");
    QTest::addColumn<quint8>("tokenLength");
    QTest::addColumn<int>("optionsListLength");
    QTest::addColumn<QList<QCoapOption::OptionName>>("optionsNames");
    QTest::addColumn<QList<quint8>>("optionsLengths");
    QTest::addColumn<QList<QByteArray>>("optionsValues");
    QTest::addColumn<QString>("payload");
    QTest::addColumn<QString>("pduHexa");

    QList<QCoapOption::OptionName> optionsNamesReply({QCoapOption::ContentFormat,
                                                          QCoapOption::MaxAge});
    QList<quint8> optionsLengthsReply({0,1});
    QList<QByteArray> optionsValuesReply({"", QByteArray::fromHex("1e")});

    QList<QCoapOption::OptionName> bigOptionsNamesReply({QCoapOption::Size1});
    QList<quint8> bigOptionsLengthsReply({26});
    QList<QByteArray> bigOptionsValuesReply({QByteArray("abcdefghijklmnopqrstuvwxyz")});

    QTest::newRow("reply_with_option_and_payload")
        << QtCoap::Content
        << QCoapMessage::NonConfirmable
        << quint16(64463)
        << QByteArray("4647f09b")
        << quint8(4)
        << 2
        << optionsNamesReply
        << optionsLengthsReply
        << optionsValuesReply
        << "Type: 1 (NON)\nCode: 1 (GET)\nMID: 56400\nToken: 4647f09b"
        << "5445fbcf4647f09bc0211eff547970653a203120284e4f4e290a436f64653a20"
           "312028474554290a4d49443a2035363430300a546f6b656e3a20343634376630"
           "3962";

    QTest::newRow("reply_without_options")
        << QtCoap::Content
        << QCoapMessage::NonConfirmable
        << quint16(64463)
        << QByteArray("4647f09b")
        << quint8(4)
        << 0
        << QList<QCoapOption::OptionName>()
        << QList<quint8>()
        << QList<QByteArray>()
        << "Type: 1 (NON)\nCode: 1 (GET)\nMID: 56400\nToken: 4647f09b"
        << "5445fbcf4647f09bff547970653a203120284e4f4e290a436f64653a20312028"
           "474554290a4d49443a2035363430300a546f6b656e3a203436343766303962";

    QTest::newRow("reply_without_payload")
        << QtCoap::Content
        << QCoapMessage::NonConfirmable
        << quint16(64463)
        << QByteArray("4647f09b")
        << quint8(4)
        << 2
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
        << 0
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
        << 1
        << bigOptionsNamesReply
        << bigOptionsLengthsReply
        << bigOptionsValuesReply
        << ""
        << "5445fbcf4647f09bdd2f0d6162636465666768696a6b6c6d6e6f707172737475"
           "767778797a";
}

void tst_QCoapReply::parseReplyPdu()
{
    QFETCH(QtCoap::StatusCode, statusCode);
    QFETCH(QCoapMessage::MessageType, type);
    QFETCH(quint16, messageId);
    QFETCH(QByteArray, token);
    QFETCH(quint8, tokenLength);
    QFETCH(int, optionsListLength);
    QFETCH(QList<QCoapOption::OptionName>, optionsNames);
    QFETCH(QList<quint8>, optionsLengths);
    QFETCH(QList<QByteArray>, optionsValues);
    QFETCH(QString, payload);
    QFETCH(QString, pduHexa);

    QCoapInternalReply reply = QCoapInternalReply::fromQByteArray(QByteArray::fromHex(pduHexa.toUtf8()));

    QCOMPARE(reply.message()->type(), type);
    QCOMPARE(reply.message()->tokenLength(), tokenLength);
    QCOMPARE(reply.statusCode(), statusCode);
    QCOMPARE(reply.message()->messageId(), messageId);
    QCOMPARE(reply.message()->token().toHex(), token);
    QCOMPARE(reply.message()->optionCount(), optionsListLength);
    for (int i = 0; i < optionsListLength; ++i) {
        QCoapOption option = reply.message()->option(i);
        QCOMPARE(option.name(), optionsNames.at(i));
        QCOMPARE(option.length(), optionsLengths.at(i));
        QCOMPARE(option.value(), optionsValues.at(i));
    }
    QCOMPARE(reply.message()->payload(), payload);
}

class QCoapReplyForTests : public QCoapReply
{
public:
    QCoapReplyForTests(const QCoapRequest &req) : QCoapReply (req)
    {
    }

    void updateFromInternalReplyForTests(const QCoapInternalReply &internal) {
        updateFromInternalReply(internal);
    }
};

void tst_QCoapReply::updateReply_data()
{
    QTest::addColumn<QString>("data");

    QTest::newRow("success") << "Data for the updating test";
}

void tst_QCoapReply::updateReply()
{
    QFETCH(QString, data);

    QCoapReplyForTests reply((QCoapRequest()));
    QCoapInternalReply internalReply;
    internalReply.message()->setPayload(data.toUtf8());
    QSignalSpy spyReplyFinished(&reply, SIGNAL(finished(QCoapReply*)));

    reply.updateFromInternalReplyForTests(internalReply);

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 1000);
    QCOMPARE(reply.readAll(), data.toUtf8());
}

QTEST_MAIN(tst_QCoapReply)

#include "tst_qcoapreply.moc"
