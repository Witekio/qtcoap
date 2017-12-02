/****************************************************************************
**
** Copyright (C) 2017 Witekio.
** Contact: https://witekio.com/contact/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:GPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
