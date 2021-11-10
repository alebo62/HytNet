#include <QCoreApplication>
#include "tcp.h"
//#include <string>

extern QString hadrr;
extern QString host;
QString str;
static quint32 radio_id;



int main(int argc, char *argv[])
{
    if(argc == 3)
    {
        hadrr = QString(argv[1]);
        radio_id =  QString(argv[2]).toUInt();

        str.setNum (radio_id >> 16);
        host.append(str).append(".");
        str.setNum ((radio_id >> 8) & 0xFF);
        host.append(str).append(".");
        str.setNum (radio_id & 0xFF);
        host.append(str);
    }
    QCoreApplication a(argc, argv);
    TCP tcp;
    return a.exec();
}
