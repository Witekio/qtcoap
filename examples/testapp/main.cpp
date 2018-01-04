#include "coaphandler.h"

#include <QCoreApplication>
#include <QCoapClient>
#include <QCoapReply>
#include <QCoapDiscoveryReply>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    CoapHandler handler;

    return a.exec();
}
