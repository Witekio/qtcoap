#ifndef QCOAPREQUEST_H
#define QCOAPREQUEST_H

#include <QUrl>

#include "qcoapmessage.h"

class QCoapRequest : public QCoapMessage
{
public:
    QCoapRequest();

    QUrl url();
    void setUrl(QUrl url);
};

#endif // QCOAPREQUEST_H
