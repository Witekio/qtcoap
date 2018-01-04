#include "coaphandler.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    CoapHandler handler;

    return a.exec();
}
