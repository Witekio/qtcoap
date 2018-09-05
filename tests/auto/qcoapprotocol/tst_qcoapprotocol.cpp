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

#include <QtCoap/qcoapprotocol.h>

class tst_QCoapProtocol : public QObject
{
    Q_OBJECT

private slots:
    void setRetransmissionSettings();
    void setBlockSize_data();
    void setBlockSize();
    void generateUniqueToken();
    void generateUniqueMessageId();
    void tokenAndMessageIdInUse();
};

void tst_QCoapProtocol::setRetransmissionSettings()
{
    QCoapProtocol protocol;

    protocol.setAckRandomFactor(4);
    QCOMPARE(protocol.ackRandomFactor(), 4);

    protocol.setAckTimeout(3000);
    QCOMPARE(protocol.ackTimeout(), 3000);

    protocol.setMaxRetransmit(10);
    QCOMPARE(protocol.maxRetransmit(), 10);
}

void tst_QCoapProtocol::setBlockSize_data()
{
    QTest::addColumn<int>("blockSizeSet");
    QTest::addColumn<int>("blockSizeExpected");

    QTest::newRow("valid_size_0")       << 0 << 0;
    QTest::newRow("valid_size_16")      << 16 << 16;
    QTest::newRow("valid_size_1024")    << 1024 << 1024;
    QTest::newRow("invalid_size_8")     << 8 << 0;
    QTest::newRow("invalid_size_350")   << 350 << 0;
    QTest::newRow("invalid_size_2048")  << 2048 << 0;
}

void tst_QCoapProtocol::setBlockSize()
{
    QFETCH(int, blockSizeSet);
    QFETCH(int, blockSizeExpected);

    QCoapProtocol protocol;
    protocol.setBlockSize(static_cast<quint16>(blockSizeSet));

    QCOMPARE(protocol.blockSize(), blockSizeExpected);
}

void tst_QCoapProtocol::generateUniqueToken()
{
    QCoapProtocol protocol;

    for (int i = 0; i < 1000; ++i)
        QVERIFY(!protocol.isTokenInUse(protocol.generateUniqueToken()));
}

void tst_QCoapProtocol::generateUniqueMessageId()
{
    QCoapProtocol protocol;

    for (int i = 0; i < 1000; ++i)
        QVERIFY(!protocol.isMessageIdInUse(protocol.generateUniqueMessageId()));
}

void tst_QCoapProtocol::tokenAndMessageIdInUse()
{
    QCoapConnection connection;
    QCoapProtocol protocol;

    QVector<QSharedPointer<QCoapReply>> replies;
    for (int i = 0; i < 20; ++i) {
        auto reply = QSharedPointer<QCoapReply>::create(QCoapRequest{});
        protocol.sendRequest(reply.data(), &connection);
        replies.append(reply);
    }

    for (auto reply : qAsConst(replies)) {
        QVERIFY(protocol.isTokenInUse(reply->request().token()));
        QVERIFY(protocol.isMessageIdInUse(reply->request().messageId()));
    }
}

QTEST_APPLESS_MAIN(tst_QCoapProtocol)

#include "tst_qcoapprotocol.moc"
