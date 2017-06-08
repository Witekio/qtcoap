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

    QUrl url;
    QCoapConnection* connection;
    QCoapReply* reply;
    QCoapRequest::QCoapRequestOperation operation;
    QCoapRequest::QCoapRequestState state;

    Q_DECLARE_PUBLIC(QCoapRequest)
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_P_H
