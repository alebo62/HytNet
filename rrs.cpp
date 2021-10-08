#include "tcp.h"
#include "messages.h"
#include "enums.h"

extern quint8       ars_answer[16];
extern quint8       ars_check[11];
extern QString      dest_rrs_ip;
extern strMsgHeader header;
extern QString      host;
extern quint8       req_id[];
extern strRMRpt 	sRegMsgReport;
extern strARSmsg    sARSmsg;
extern QByteArray   tcp_tx, ba;

extern void checksum(unsigned char*);
extern void crc(unsigned char*, int);


QString         ars_addr;
QByteArray      ba_3002;
unsigned char   buf_tx[64];


void TCP::rcv_udpRRS()
{
    ba_3002.resize(static_cast<int>(udpRRS_3002.pendingDatagramSize()));
    udpRRS_3002.readDatagram(ba_3002.data(), ba_3002.size());
    qDebug() << "rrs " << ba_3002.toHex();

    if(ba_3002.at(2) == 3)// registration
    {
        sARSmsg.userId[0] = 0;// номер сети не нужен
        sARSmsg.userId[1] = ba_3002.at(6);
        sARSmsg.userId[2] = ba_3002.at(7);
        sARSmsg.userId[3] = ba_3002.at(8);
        sARSmsg.arsMsgType = RegMessage;
        tcp_tx.resize(sizeof(sARSmsg));
        memmove(tcp_tx.data(), &sARSmsg, sizeof(sARSmsg));
        if(tcp_srv.state() ==  QAbstractSocket::ConnectedState)
            tcp_srv.write(tcp_tx, sizeof(sARSmsg));
    }
    else if(ba_3002.at(2) == 1) // deregistration
    {
        sARSmsg.userId[0] = 0;// номер сети не нужен
        sARSmsg.userId[1] = ba_3002.at(6);
        sARSmsg.userId[2] = ba_3002.at(7);
        sARSmsg.userId[3] = ba_3002.at(8);
        sARSmsg.arsMsgType = DeregMessage;
        //memcpy(sARSmsg.req_id, req_id, 4);
        tcp_tx.resize(sizeof(sARSmsg));
        memmove(tcp_tx.data(), &sARSmsg, sizeof(sARSmsg));
        if(tcp_srv.state() ==  QAbstractSocket::ConnectedState)
            tcp_srv.write(tcp_tx, sizeof(sARSmsg));
    }
    else if(ba_3002.at(2) == 0x82) // check answer
    {
        //sARSmsg.userId[0] = 0x;// номер сети не нужен
        sARSmsg.userId[1] = ba_3002.at(6);
        sARSmsg.userId[2] = ba_3002.at(7);
        sARSmsg.userId[3] = ba_3002.at(8);
        sARSmsg.arsMsgType = RegAck;
        sARSmsg.csbk = 0;
        memcpy(sARSmsg.req_id, req_id, 4);
        tcp_tx.resize(sizeof(sARSmsg));
        memmove(tcp_tx.data(), &sARSmsg, sizeof(sARSmsg));
        if(tcp_srv.state() ==  QAbstractSocket::ConnectedState)
            tcp_srv.write(tcp_tx, sizeof(sARSmsg));
    }
}

void TCP::rcv_tcpRRS()
{

        memcpy(req_id, ba.data() + 10, 4); //  add 05.04
//                if(ba.at(sizeof(header)) == SuccessReg)
//                {
//                    ars_answer[6] = ba.at(6);
//                    ars_answer[7] = ba.at(7);
//                    ars_answer[8] = ba.at(8);
//                    ars_answer[9] = 0;

        ars_addr = dest_rrs_ip + QString::number(ars_check[6]) + ".";
        ars_addr += QString::number(ars_check[7]) + ".";
        ars_addr += QString::number(ars_check[8]);

//                    crc(ars_answer, 16);
//                    udpRRS_3002.writeDatagram((char*)ars_answer, 16, QHostAddress(host), RRS);
              udpRRS_3002.flush();

//                }
        if(ba.at(sizeof(header)) == FailureReg)
        {
            ars_answer[6] = ba.at(6);
            ars_answer[7] = ba.at(7);
            ars_answer[8] = ba.at(8);
            ars_answer[9] = 1;

            ars_addr = dest_rrs_ip + QString::number(ars_check[6]) + ".";
            ars_addr += QString::number(ars_check[7]) + ".";
            ars_addr += QString::number(ars_check[8]);

            crc(ars_answer, 16);
            udpRRS_3002.writeDatagram((char*)ars_answer, 16, QHostAddress(ars_addr), RRS);
            udpRRS_3002.flush();
        }
        else if(ba.at(sizeof(header)) == 2)//QueryMessage)
        {
            ars_check[6] = ba.at(6);
            ars_check[7] = ba.at(7);
            ars_check[8] = ba.at(8);

            ars_addr = dest_rrs_ip + QString::number(ars_check[6]) + ".";
            ars_addr += QString::number(ars_check[7]) + ".";
            ars_addr += QString::number(ars_check[8]);

            crc(ars_check, 11);
            udpRRS_3002.writeDatagram((char*)ars_check, 11, QHostAddress(ars_addr), RRS);
            udpRRS_3002.flush();
        }
        else if(ba.at(sizeof(header)) == Unknown)
        {
#ifdef DBG_BA
            qDebug() << "Unknown rrs ";
#endif
        }

}
