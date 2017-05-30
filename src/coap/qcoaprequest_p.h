#ifndef QCOAPREQUEST_P_H
#define QCOAPREQUEST_P_H

#include "qcoaprequest.h"
#include "qcoapmessage_p.h"

QT_BEGIN_NAMESPACE

class QCoapRequestPrivate : public QCoapMessagePrivate
{
public:
    QCoapRequestPrivate();
    ~QCoapRequestPrivate();

    QUrl url_p;
    QCoapConnection* connection_p;
    QCoapReply* reply_p;
    QCoapRequest::QCoapRequestOperation operation_p;

    Q_DECLARE_PUBLIC(QCoapRequest)
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_P_H
