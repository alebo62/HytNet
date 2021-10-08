#include "tcp.h"
#include "messages.h"

extern QString host, hadrr;
extern quint16 hyterus_port;
extern unsigned char conn[];
extern strRMRpt  sRegMsgReport;
extern quint32   ping_counter;
extern quint32 Radio_Reg_State;
extern quint16 msg_cnt;
extern quint32 state;

void TCP::rad_conn_tim_slot()
{
    qDebug() << "connect";
    udpRCP_3005.writeDatagram((char*)conn, 12, QHostAddress(host), RCP);
}

void TCP::tcp_conn_tim_slot()
{
    if(tcp_srv.state() ==  QAbstractSocket::UnconnectedState )
        tcp_srv.connectToHost(hadrr, hyterus_port);
    else if(tcp_srv.state() ==  QAbstractSocket::ConnectedState)
    {
        tcp_conn_tim.stop();
        tcp_srv.write((char*)&sRegMsgReport, sizeof(sRegMsgReport));
        tcp_srv.flush();
        qDebug() << "RegMsgReport send";
    }
}

void TCP::ping_tim_slot()
{
    ping_counter++;
#ifdef DBG
    qDebug() << ping_counter;
#endif
    if(ping_counter == 3)
    {
        ping_tim.stop();
        tcp_srv.close();
    }
}

void TCP::reload_tim_slot()
{
    reload_tim.stop();
    state = 0;
    udpRCP_3005.writeDatagram((char*)conn, 12, QHostAddress(host), RCP);
    Radio_Reg_State = INIT_STATE;
    msg_cnt = 0;
}
