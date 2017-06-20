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
        IFMATCH         = 1,
        URIHOST         = 3,
        ETAG            = 4,
        IFNONEMATCH     = 5,
        OBSERVE         = 6,
        URIPORT         = 7,
        LOCATIONPATH    = 8,
        URIPATH         = 11,
        CONTENTFORMAT   = 12,
        MAXAGE          = 14,
        URIQUERY        = 15,
        ACCEPT          = 17,
        LOCATIONQUERY   = 20,
        BLOCK2          = 23,
        BLOCK1          = 27,
        SIZE2           = 28,
        PROXYURI        = 35,
        PROXYSCHEME     = 39,
        SIZE1           = 60
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
