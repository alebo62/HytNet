#include <QCoreApplication>
#include "tcp.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TCP tcp;
    return a.exec();
}
