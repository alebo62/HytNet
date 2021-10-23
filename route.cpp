#include <QProcess>
#include <QThread>
#include <QDebug>
#include "tcp.h"

static QProcess proc;
static QProcess proc1;
static QProcess proc2;
static QStringList sl;
static bool need_wait_usb01 = true;
static QString output;

class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};
// 02c00081ec5b1abb  nanopi
//94007868060c443f03ce  orangepi

void route_rpi()
{
    QStringList sl;
    do
    {
        Sleeper::sleep(3);
        proc.start("cat /proc/cpuinfo");
        proc.waitForFinished();
        sl = ((QString)proc.readAllStandardOutput()).split('\n');
    }
    while(sl[sl.size() - 3].contains("5883") == false);            //  rpi  97  white box

//    while(sl[sl.size() - 3].contains("2c60") == false);            //  rpi 144  2010
//    while(sl[sl.size() - 3].contains("ba37") == false);            //  rpi 150  2004
//    while(sl[sl.size() - 3].contains("0dd7") == false);            //  rpi 151  2012
//    while(sl[sl.size() - 3].contains("f252") == false);            //  rpi 152  2005
//    while(sl[sl.size() - 3].contains("213a") == false);            //  rpi 154  2007
//    while(sl[sl.size() - 3].contains("0044") == false);            //  rpi 156  2006
//    while(sl[sl.size() - 3].contains("46de") == false);            //  rpi 157  2008
//    while(sl[sl.size() - 3].contains("e65b") == false);            //  rpi 158  2003
//    while(sl[sl.size() - 3].contains("212b") == false);            //  rpi 159  2009
//    while(sl[sl.size() - 3].contains("28ea") == false);            //  rpi 162  2011
//    while(sl[sl.size() - 3].contains("7f91") == false);            //  rpi 161  2002
//    while(sl[sl.size() - 3].contains("227f") == false);            //  rpi 160  2001

#ifdef DBG
    qDebug() << "usb and serial ok";
#endif
    proc.start("sudo route -n");
    proc.waitForFinished();
    sl = ((QString)proc.readAllStandardOutput()).split('\n');
    for(int i = 0; i < sl.size(); i++)
    {
        if(sl[i].contains("192.168.10.0") && sl[i].endsWith("eth1"))//enx00000c000034"))
        {
            proc.start("sudo route add -net 10.0.0.0/8 gw 192.168.10.1 eth1");
            proc.waitForFinished();
        }
    }
#ifdef DBG
    qDebug() << "end route";
#endif
}
