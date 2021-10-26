#include "tcp.h"
#include "messages.h"
#include "enums.h"

extern void checksum(unsigned char*);
extern void crc(quint8*, int);

extern quint8           ars_answer[16];
extern quint8           ars_check[11];
extern unsigned char    buf_tx[];
extern hrnp_t           call_req;
extern QString          dest_rrs_ip;
extern strMsgHeader     header;
extern QString          hadrr, host;
extern quint16          hyterus_port;
extern quint16          msg_cnt;
extern hrnp_t           ptt_release_req;
extern hrnp_t           ptt_press_req;
extern rtp_header       rtp_hdr;
extern quint32          Radio_Reg_State;
extern hrnp_t           restart;
extern hrnp_t           remote_monitor;
extern hrnp_t           rtp_pld_req;
extern strMRRpt         sMRRpt;
extern strCtrlRpl       sCtrlReply;
extern strRldRpl        sReloadReplay;
extern strCCRpt         sChanChangeReport;
extern strCCRpl         sChanChangeReply;
extern strCRpt          sCallReport;
extern strCSRp          sCallStopReply;
extern strCSRp          sStopCallReply;
extern strCRp           sCallReply;
//extern strCallControl   sCallControl;
extern strRMRpt         sRegMsgReport;
extern strMsgType_Ping  sMsgType_Ping;
extern hrnp_t           zone_ch;

QByteArray  ba;
bool        conn_state_kupol;
quint8      last_abonent[4];
quint8      mu_law;
quint32     ping_counter;
quint32     Radio_Reg_State;
quint8      req_id[4];//        msg_cnt++;
QByteArray  tcp_tx;

void TCP::tcp_receive()
{
    ba = tcp_srv.read(256);
    ping_tim.start();

    switch (ba.at(0)) {
    case 4: // ping message
        //qDebug() << "ping send";
        tcp_srv.write(reinterpret_cast<char*>(&sMsgType_Ping), sizeof(sMsgType_Ping));
        break;
    case 1: // control messages
        qDebug() << "tcp:" << ba.toHex() << Radio_Reg_State;
        rcv_tcpRCP();
        break;
    case 3: // location messages
        qDebug() << "tcp:" << ba.toHex();
        if(Radio_Reg_State == READY)
        {
            rcv_tcpGPS();
        }
        break;
    case 0: // rrs messages
        qDebug() << "tcp:" << ba.toHex();
        if(Radio_Reg_State == READY)
        {
            rcv_tcpRRS();
        }
        break;
    case 2: // text messages
        qDebug() << "tcp:" << ba.toHex();
        if(Radio_Reg_State == READY)
        {
            rcv_tcpTMS();
        }
        break;
    default:
        break;
    }
}

void TCP::tcp_conn()
{
    qDebug() << "connect";
    tcp_tx.resize(sizeof(sMRRpt));
    memmove(tcp_tx.data(), &sMRRpt, sizeof(sMRRpt));
    tcp_srv.write(tcp_tx, sizeof(sMRRpt));
    tcp_srv.flush();
    qDebug() << "MRRpt send";

}

void TCP::tcp_disconn()
{
    ping_tim.stop();
    tcp_srv.abort();
    disconnect(&udp_srv, SIGNAL(readyRead()),this, SLOT(udp_srv_slot()));
    tcp_conn_tim.start();
}


