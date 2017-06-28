#include "qcoapresource.h"
#include "qcoapresource_p.h"
#include <QDebug>

QT_BEGIN_NAMESPACE

QCoapResourcePrivate::QCoapResourcePrivate() :
    path(""),
    title(""),
    observable(false),
    resourceType(""),
    interface(""),
    maximumSize(-1),
    contentFormat(0)
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

int QCoapResource::maximumSize() const
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

void QCoapResource::setMaximumSize(int maximumSize)
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



QT_END_NAMESPACE
