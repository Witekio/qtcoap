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
        IfMatchOption         = 1,
        UriHostOption         = 3,
        EtagOption            = 4,
        IfNoneMatchOption     = 5,
        ObserveOption         = 6,
        UriPortOption         = 7,
        LocationPathOption    = 8,
        UriPathOption         = 11,
        ContentFormatOption   = 12,
        MaxAgeOption          = 14,
        UriQueryOption        = 15,
        AcceptOption          = 17,
        LocationQueryOption   = 20,
        Block2Option          = 23,
        Block1Option          = 27,
        Size2Option           = 28,
        ProxyUriOption        = 35,
        ProxySchemeOption     = 39,
        Size1Option           = 60
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
