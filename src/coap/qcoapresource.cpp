#include "qcoapresource.h"
#include "qcoapresource_p.h"

QCoapResourcePrivate::QCoapResourcePrivate()
{
}

QCoapResourcePrivate::~QCoapResourcePrivate()
{
}

QCoapResource::QCoapResource() :
    d_ptr(new QCoapResourcePrivate)
{
}

QString QCoapResource::path() const
{
    return d_ptr->path;
}

QString QCoapResource::title() const
{
    return d_ptr->title;
}

bool QCoapResource::observable() const
{
    return d_ptr->observable;
}

QString QCoapResource::resourceType() const
{
    return d_ptr->resourceType;
}

QString QCoapResource::interface() const
{
    return d_ptr->interface;
}

uint QCoapResource::maximumSize() const
{
    return d_ptr->maximumSize;
}

uint QCoapResource::contentFormat() const
{
    return d_ptr->contentFormat;
}

void QCoapResource::setPath(const QString& path)
{
    if (d_ptr->path == path)
        return;

    d_ptr->path = path;
}

void QCoapResource::setTitle(const QString& title)
{
    if (d_ptr->title == title)
        return;

    d_ptr->title = title;
}

void QCoapResource::setObservable(bool observable)
{
    if (d_ptr->observable == observable)
        return;

    d_ptr->observable = observable;
}

void QCoapResource::setResourceType(const QString& resourceType)
{
    if (d_ptr->resourceType == resourceType)
        return;

    d_ptr->resourceType = resourceType;
}

void QCoapResource::setInterface(const QString& interface)
{
    if (d_ptr->interface == interface)
        return;

    d_ptr->interface = interface;
}

void QCoapResource::setMaximumSize(uint maximumSize)
{
    if (d_ptr->maximumSize == maximumSize)
        return;

    d_ptr->maximumSize = maximumSize;
}

void QCoapResource::setContentFormat(uint contentFormat)
{
    if (d_ptr->contentFormat == contentFormat)
        return;

    d_ptr->contentFormat = contentFormat;
}

QList<QCoapResource> QCoapResource::fromCoreLinkList(const QByteArray& data)
{
    QList<QCoapResource> resourceList;

    QList<QByteArray> links = data.split(',');
    for (QByteArray link : links)
    {
        QCoapResource resource;
        QList<QByteArray> parameterList = link.split(';');
        for (QByteArray parameter : parameterList)
        {
            int length = parameter.length();
            if (link.startsWith('<'))
                resource.setPath(QString(parameter).mid(1, length-2));
            else if (link.startsWith("title="))
                resource.setTitle(QString(parameter).mid(7, length-2));
            else if (link.startsWith("rt="))
                resource.setResourceType(QString(parameter).mid(4, length-2));
            else if (link.startsWith("if="))
                resource.setInterface(QString(parameter).mid(4, length-2));
            else if (link.startsWith("sz="))
                resource.setMaximumSize(parameter.mid(3, length-1).toUInt());
            else if (link.startsWith("ct="))
                resource.setContentFormat(parameter.mid(3, length-1).toUInt());
            else if (link == "obs")
                resource.setObservable(true);
        }

        resourceList.push_back(resource);
    }

    return resourceList;
}
