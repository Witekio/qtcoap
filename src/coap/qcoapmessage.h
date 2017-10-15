/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:LGPL3$
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

#ifndef QCOAPMESSAGE_H
#define QCOAPMESSAGE_H

#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapoption.h>
#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QCoapMessagePrivate;
class Q_COAP_EXPORT QCoapMessage
{
public:
    enum MessageType {
        Confirmable,
        NonConfirmable,
        Acknowledgement,
        Reset
    };

    QCoapMessage();
    QCoapMessage(const QCoapMessage &other);
    virtual ~QCoapMessage();

    void swap(QCoapMessage &other) Q_DECL_NOTHROW;
    QCoapMessage &operator=(const QCoapMessage &other);
    QCoapMessage &operator=(QCoapMessage &&other) Q_DECL_NOTHROW;

    quint8 version() const;
    MessageType type() const;
    QByteArray token() const;
    quint8 tokenLength() const;
    quint16 messageId() const;
    QByteArray payload() const;
    QCoapOption option(int index) const;
    QList<QCoapOption> optionList() const;
    int optionsLength() const;
    void setVersion(quint8 version);
    void setType(const MessageType &type);
    void setToken(const QByteArray &token);
    void setMessageId(quint16);
    void setPayload(const QByteArray &payload);

    void addOption(QCoapOption::OptionName name, const QByteArray &value = QByteArray());
    virtual void addOption(const QCoapOption &option);
    QCoapOption findOptionByName(QCoapOption::OptionName name);
    void removeOption(const QCoapOption &option);
    void removeOptionByName(QCoapOption::OptionName name);
    void removeAllOptions();

protected:
    explicit QCoapMessage(QCoapMessagePrivate &dd);

    QCoapMessagePrivate* d_ptr;
};

//Q_DECLARE_SHARED(QCoapMessage)
Q_DECLARE_METATYPE(QCoapMessage::MessageType)

QT_END_NAMESPACE

#endif // QCOAPMESSAGE_H
