#ifndef QCOAPOPTION_H
#define QCOAPOPTION_H

#include <QObject>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QCoapOptionPrivate;
class QCoapOption
{
public:
    enum QCoapOptionName {
        IfMatchCoapOption         = 1,
        UriHostCoapOption         = 3,
        EtagCoapOption            = 4,
        IfNoneMatchCoapOption     = 5,
        ObserveCoapOption         = 6,
        UriPortCoapOption         = 7,
        LocationPathCoapOption    = 8,
        UriPathCoapOption         = 11,
        ContentFormatCoapOption   = 12,
        MaxAgeCoapOption          = 14,
        UriQueryCoapOption        = 15,
        AcceptCoapOption          = 17,
        LocationQueryCoapOption   = 20,
        Block2CoapOption          = 23,
        Block1CoapOption          = 27,
        Size2CoapOption           = 28,
        ProxyUriCoapOption        = 35,
        ProxySchemeCoapOption     = 39,
        Size1CoapOption           = 60,
        InvalidCoapOption
    };

    QCoapOption(QCoapOptionName name, const QByteArray& value);

    QByteArray value() const;
    quint8 length() const;
    QCoapOptionName name() const;

    bool operator==(const QCoapOption& other) const;

protected:
    QCoapOptionPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QCOAPOPTION_H
