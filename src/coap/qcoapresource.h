#ifndef QCOAPRESOURCE_H
#define QCOAPRESOURCE_H

#include <QObject>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QCoapResourcePrivate;
class QCoapResource
{
public:
    explicit QCoapResource();

    QString path() const;
    QString title() const;
    bool observable() const;
    QString resourceType() const;
    QString interface() const;
    int maximumSize() const;
    uint contentFormat() const;

    void setPath(const QString& path);
    void setTitle(const QString& title);
    void setObservable(bool observable);
    void setResourceType(const QString& resourceType);
    void setInterface(const QString& interface);
    void setMaximumSize(int maximumSize);
    void setContentFormat(uint contentFormat);

private:
    QCoapResourcePrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QCOAPRESOURCE_H
