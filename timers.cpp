#include "tcp.h"
#include "messages.h"
#include "enums.h"

extern QString host, hadrr;
extern quint16 hyterus_port;
extern unsigned char conn[];
extern strRMRpt  sRegMsgReport;
extern strCtrlRpl sCtrlReply;
extern quint32   ping_counter;
extern quint32 Radio_Reg_State;
extern quint16 msg_cnt;
extern quint32 state;
extern quint32 check_online;
extern quint32 enable_online;
extern quint32 disable_online;
extern quint32 monitor_online;
void TCP::rad_conn_tim_slot()
{
    qDebug() << "connect";
    udpRCP_3005.writeDatagram((char*)conn, 12, QHostAddress(host), RCP);
    udpRCP_3005.flush();
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
#ifdef DBG
    qDebug() << "reload tim slot";
#endif
}

void TCP::radio_check_tim_slot()
{
    if(tcp_srv.state() ==  QAbstractSocket::ConnectedState)
    {
        check_online = 0;
        sCtrlReply.result = 1; // fail
        sCtrlReply.req_type = 0;// radio check
        radio_check_tim.stop();
        tcp_srv.write((char*)&sCtrlReply, sizeof(sCtrlReply));
        tcp_srv.flush();
#ifdef DBG
        qDebug() << "radio check timer";
#endif
    }
}

void TCP::monitor_tim_slot()
{
    if(tcp_srv.state() ==  QAbstractSocket::ConnectedState)
    {
        monitor_online = 0;
        sCtrlReply.result = 1; // fail
        sCtrlReply.req_type = 3;// monitor radio
        monitor_tim.stop();
        tcp_srv.write((char*)&sCtrlReply, sizeof(sCtrlReply));
        tcp_srv.flush();
#ifdef DBG
        qDebug() << "radio monitor timer";
#endif
    }

}

void TCP::rx_tim_slot()
{
    Radio_Reg_State = READY;
    rx_tim.stop();
#ifdef DBG
    qDebug() << "rx timer slot";
#endif
}

void TCP::tx_tim_slot()
{
    Radio_Reg_State = READY;
    tx_tim.stop();
#ifdef DBG
    qDebug() << "tx timer slot";
#endif
}
