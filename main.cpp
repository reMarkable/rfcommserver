#include <QCoreApplication>
#include "rfcommserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    RFCommServer server;
    server.start();

    qDebug() << "SERVER STARTED";
    return a.exec();
}
