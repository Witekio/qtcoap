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

#ifndef QCOAPMESSAGE_H
#define QCOAPMESSAGE_H

#include <QtCore/qglobal.h>
#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapoption.h>
#include <QtCore/qobject.h>
#include <QtCore/qvector.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QCoapMessagePrivate;
class Q_COAP_EXPORT QCoapMessage
{
public:
    enum MessageType {
        Confirmable,
        NonConfirmable,
        Acknowledgment,
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
    void setVersion(quint8 version);
    void setType(const MessageType &type);
    void setToken(const QByteArray &token);
    void setMessageId(quint16);
    void setPayload(const QByteArray &payload);

    QCoapOption option(int index) const;
    //! TODO: Add the possibility to retrieve multiple QCoapOption with the same OptionName.
    QCoapOption option(QCoapOption::OptionName name) const;
    QVector<QCoapOption>::const_iterator findOption(QCoapOption::OptionName name) const;
    bool hasOption(QCoapOption::OptionName name) const;
    const QVector<QCoapOption> &options() const;
    int optionCount() const;
    void addOption(QCoapOption::OptionName name, const QByteArray &value = QByteArray());
    virtual void addOption(const QCoapOption &option);
    void removeOption(const QCoapOption &option);
    void removeOption(QCoapOption::OptionName name);
    void removeAllOptions();

protected:
    explicit QCoapMessage(QCoapMessagePrivate &dd);

    QSharedDataPointer<QCoapMessagePrivate> d_ptr;

    //! For QSharedDataPointer
    inline QCoapMessagePrivate *d_func();
    const QCoapMessagePrivate *d_func() const
    {
        return d_ptr.constData();
    }
};

Q_DECLARE_SHARED(QCoapMessage)
Q_DECLARE_METATYPE(QCoapMessage)
Q_DECLARE_METATYPE(QCoapMessage::MessageType)

QT_END_NAMESPACE

#endif // QCOAPMESSAGE_H
