#include "tcp.h"
#include "messages.h"
#include "enums.h"


extern QByteArray   ba;
extern QString      dest_rrs_ip;
extern quint8       txt_msg_tx[255];
extern strRMRpt     sRegMsgReport;
extern strTxtMsg_SimpMsg sTxtMsg_SimpMsg;

/*  MsgHdr(0x09)
    Opcode    0x80 0xB1(SendGroupMsg)    0x80 0xA1(SendPrivateMsg) (1,2)
    #of bytes  (3,4)
    Payload   RequestID[4]+DestID[4]+SrcID[4]+TMData[n] (5..8  9..12 13..16 17...)
    Checksum        17 + #of bytes
    MsgEnd (0x03)   18 + #of bytes

*/

extern void crc(unsigned char*, int);

QString     tms_addr;
QByteArray  ba_3004;

void TCP::rcv_udpTMS()
{
    ba_3004.resize(static_cast<int>(udpTMS_3004.pendingDatagramSize()));
    udpTMS_3004.readDatagram(ba_3004.data(), ba_3004.size());
    qDebug() << "txt " << ba_3004.toHex();

    sTxtMsg_SimpMsg.SenderId[0] = 0;// номер сети не нужен
    sTxtMsg_SimpMsg.SenderId[1] = ba_3004.at(14);
    sTxtMsg_SimpMsg.SenderId[2] = ba_3004.at(15);
    sTxtMsg_SimpMsg.SenderId[3] = ba_3004.at(16);


    sTxtMsg_SimpMsg.ReceiverType = 0;
    sTxtMsg_SimpMsg.SequenceNumber = ba_3004.at(8);
    sTxtMsg_SimpMsg.IsAckRequired = false;
    if(ba_3004.at(2) == 0xA1)
    {
        sTxtMsg_SimpMsg.ReceiverType = eDevice;
        memcpy(sTxtMsg_SimpMsg.ReceiverId, sRegMsgReport.radio_id , 4);
    }
    else if(ba_3004.at(2) == 0xB1)
    {
        sTxtMsg_SimpMsg.ReceiverType = eTalkGroup;
        memcpy(sTxtMsg_SimpMsg.ReceiverId, ba_3004.data() + 10 , 3);
    }

    sTxtMsg_SimpMsg.header.payloadLength[1] = 9 + 3 + (ba_3004.at(4) - 12);           // ?
    memcpy(sTxtMsg_SimpMsg.Text, ba_3004.data() + 17, ba_3004.at(4) - 12);            // ?
    tcp_srv.write(reinterpret_cast<char*>(&sTxtMsg_SimpMsg), 16 + ba_3004.at(4) - 12);// ?
}

/*
 * IronAgentMsgType  1 байт (0x02)
 * PayloadLength     2 байта
 * Reserved          1 byte

 *  TxtMsgType      1 байт (4)
 *  int ReceiverId  4 байта	(5..8)
 *  int SenderId    4 байта	(9..12)
 *
 Payload:
    byte ReceiverType – Тип получателя,(13)
    bool IsAckRequired – Требуется ли подтверждение,(14)
    byte SequenceNumber – Последовательный номер,(15)
    byte[] Text – Текстовое сообщение (16...)

 */

void TCP::rcv_tcpTMS()
{
    qDebug() << "txt msg " << ba.toHex();
    switch (ba.at(4)) {
    case TmsServAvail:
        // "02 00-09 00 00 00-00-00-33 00000001"
        break;
    case TmsAck:

        break;
    case SimpleTextMessage:  // 02001600 03 00-00-00-33 00 00 00 01 01 01 01(15)  "6800-6500-6c00-6c00-6f00" hello
        if(ba.at(13) == eDevice)
        {
            txt_msg_tx[2] = 0xA1;
            if(ba.at(14) == false)//ack requred
            {
                txt_msg_tx[2] &= 0x03;
            }
            else
            {
                txt_msg_tx[1] |= 0x80;
            }
        }
        else if(ba.at(13) == eTalkGroup)
        {
            txt_msg_tx[2] = 0xB1;
        }
        memcpy(&txt_msg_tx[17], ba.data() + 16, static_cast<quint32>(ba.at(2) - 12));

        txt_msg_tx[5] = 0;  // req id
        txt_msg_tx[6] = 0;
        txt_msg_tx[7] = 0;
        txt_msg_tx[8] = ba.at(15);

        txt_msg_tx[9] =  dest_rrs_ip.toInt(); // dest
        txt_msg_tx[10] = ba.at(6);
        txt_msg_tx[11] = ba.at(7);
        txt_msg_tx[12] = ba.at(8);

        tms_addr =  dest_rrs_ip + QString::number(txt_msg_tx[10]) + ".";
        tms_addr += QString::number(txt_msg_tx[11]) + ".";
        tms_addr += QString::number(txt_msg_tx[12]);

        txt_msg_tx[13] =  dest_rrs_ip.toInt();  // source
        txt_msg_tx[14] =  sRegMsgReport.radio_id[1];
        txt_msg_tx[15] =  sRegMsgReport.radio_id[2];
        txt_msg_tx[16] =  sRegMsgReport.radio_id[3];

        txt_msg_tx[4] = (ba.at(2) - 12) + 12;

        txt_msg_tx[17 + ba.at(2) - 2] = 0x00;
        txt_msg_tx[17 + ba.at(2) - 1] = 0x03;

        crc(txt_msg_tx , 17 + ba.at(2));//!!!!!!!!!!!!!!!

        //qDebug() << test_ba.toHex();

         udpTMS_3004.writeDatagram((char*)txt_msg_tx, 17 + ba.at(2), QHostAddress(tms_addr), TMS);
         udpTMS_3004.flush();
        break;
    default:
        break;
    }
    //  7e0400002010000e 0029 696d 09 80a1 0016 00000001 0a000033 0a000001 6800-6500-6c00-6c00-6f00 9e 00
    //  7e0400002010000e 0029 0000 09 80a1 0016 68b10600 0003 007e040000201000020018000002c4000500

}
