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

/*!
    \class QCoapResource
    \brief The QCoapResource class holds informations about a discovered
    resource.

    \reentrant

    The QCoapRequest contains data as the path and title of the resource
    and other ancillary informations.

    \sa QCoapDiscoveryReply
*/

/*!
    Constructs a new QCoapResource.
 */
QCoapResource::QCoapResource() :
    d_ptr(new QCoapResourcePrivate)
{
}

/*!
    Returns the path of the resource.

    \sa setPath()
 */
QString QCoapResource::path() const
{
    return d_ptr->path;
}

/*!
    Returns the title of the resource.

    \sa setTitle()
 */
QString QCoapResource::title() const
{
    return d_ptr->title;
}

/*!
    Returns true if the resource is observable

    \sa setObservable()
 */
bool QCoapResource::observable() const
{
    return d_ptr->observable;
}

/*!
    Returns the type of the resource.

    \sa setResourceType()
 */
QString QCoapResource::resourceType() const
{
    return d_ptr->resourceType;
}

/*!
    Returns the interface of the resource.

    \sa setInterface()
 */
QString QCoapResource::interface() const
{
    return d_ptr->interface;
}

/*!
    Returns the maximum size of the resource.

    \sa setMaximumSize()
 */
int QCoapResource::maximumSize() const
{
    return d_ptr->maximumSize;
}

/*!
    Returns the content format of the resource.

    \sa setContentFormat()
 */
uint QCoapResource::contentFormat() const
{
    return d_ptr->contentFormat;
}

/*!
    Sets the path of the resource.

    \sa path()
 */
void QCoapResource::setPath(const QString& path)
{
    d_ptr->path = path;
}

/*!
    Sets the title of the resource.

    \sa title()
 */
void QCoapResource::setTitle(const QString& title)
{
    d_ptr->title = title;
}

/*!
    Set it to true to indicate that the resource
    is observable.

    \sa observable()
 */
void QCoapResource::setObservable(bool observable)
{
    d_ptr->observable = observable;
}

/*!
    Sets the resource type.

    \sa resourceType()
 */
void QCoapResource::setResourceType(const QString& resourceType)
{
    d_ptr->resourceType = resourceType;
}

/*!
    Sets the interface of the resource.

    \sa interface()
 */
void QCoapResource::setInterface(const QString& interface)
{
    d_ptr->interface = interface;
}

/*!
    Sets the maximum size of the resource.

    \sa maximumSize()
 */
void QCoapResource::setMaximumSize(int maximumSize)
{
    d_ptr->maximumSize = maximumSize;
}

/*!
    Sets the content format of the resource.

    \sa contentFormat()
 */
void QCoapResource::setContentFormat(uint contentFormat)
{
    d_ptr->contentFormat = contentFormat;
}

QT_END_NAMESPACE
