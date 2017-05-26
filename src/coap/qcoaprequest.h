#ifndef QCOAPREQUEST_H
#define QCOAPREQUEST_H

#include <QUrl>

#include "qcoapmessage.h"

class QCoapRequest : public QCoapMessage
{
public:
    QCoapRequest(const QUrl& url = QUrl());

    QUrl url() const;
    void setUrl(const QUrl& url);
};

#endif // QCOAPREQUEST_H
