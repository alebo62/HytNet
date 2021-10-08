#include "tcp.h"
#include "messages.h"
#include "enums.h"

extern QByteArray ba;

QByteArray ba_3004;
void TCP::rcv_udpTMS()
{
    ba_3004.resize(static_cast<int>(udpTMS_3004.pendingDatagramSize()));
    udpTMS_3004.readDatagram(ba_3004.data(), ba_3004.size());
    qDebug() << "txt " << ba_3004.toHex();
}

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
            txt_msg_tx.pep.opcode[1] = 0xA1;
            if(ba.at(14) == false)//ack requred
            {
                txt_msg_tx.pep.opcode[0] &= 0x03;
            }
            else
            {
                txt_msg_tx.pep.opcode[0] |= 0x80;
            }
        }
        else if(ba.at(13) == eTalkGroup)
        {
            txt_msg_tx.pep.opcode[1] = 0xB1;
        }
        memcpy(txt_msg_tx.pep.pld + 12, ba.data() + 16, static_cast<quint32>(ba.at(2) - 12));
        txt_msg_tx.pep.pld[0] = 0;  // req id
        txt_msg_tx.pep.pld[1] = 0;
        txt_msg_tx.pep.pld[2] = 0;
        txt_msg_tx.pep.pld[3] = ba.at(15);

        txt_msg_tx.pep.pld[4] = 10; // dest
        txt_msg_tx.pep.pld[5] = ba.at(6);
        txt_msg_tx.pep.pld[6] = ba.at(7);
        txt_msg_tx.pep.pld[7] = ba.at(8);

        txt_msg_tx.pep.pld[8] = 10;  // source
        txt_msg_tx.pep.pld[9] =  sRegMsgReport.radio_id[1];
        txt_msg_tx.pep.pld[10] = sRegMsgReport.radio_id[2];
        txt_msg_tx.pep.pld[11] = sRegMsgReport.radio_id[3];
        msg_cnt++;
        txt_msg_tx.packet_num[0] = msg_cnt >> 8; // page 230  pcm pmu  pma  0 8 0x78
        txt_msg_tx.packet_num[1] = msg_cnt & 0xFF;

        txt_msg_tx.pep.num_of_bytes[1] = (ba.at(2) - 12) + 12;
        txt_msg_tx.length[1] = 19 + txt_msg_tx.pep.num_of_bytes[1];

        txt_msg_tx.pep.Checksum = 0x00;
        txt_msg_tx.pep.MsgEnd = 0x03;
        memcpy(buf_tx,      reinterpret_cast<char*>(&txt_msg_tx), 17 );
        memcpy(buf_tx + 17, reinterpret_cast<char*>(txt_msg_tx.pep.pld), txt_msg_tx.pep.num_of_bytes[1]);
        //memcpy(buf_tx + 17 + txt_msg_tx.pep.num_of_bytes[1], reinterpret_cast<char*>(&txt_msg_tx + 21), 2);
        buf_tx[17 + txt_msg_tx.pep.num_of_bytes[1]    ] = 0x00;
        buf_tx[17 + txt_msg_tx.pep.num_of_bytes[1] + 1] = 0x03;
        checksum(buf_tx);

        //test_ba.resize(txt_msg_tx.length[1]);
        //memcpy(test_ba.data(), (quint8*)&buf_tx, txt_msg_tx.length[1] );
        //qDebug() << test_ba.toHex();

        udpRCP_3005.writeDatagram((char*)buf_tx, txt_msg_tx.length[1], QHostAddress(host), RCP);
        udpRCP_3005.flush();
        break;
    default:
        break;
    }
    //  7e0400002010000e 0029 696d 09 80a1 0016 00000001 0a000033 0a000001 6800-6500-6c00-6c00-6f00 9e 00
    //  7e0400002010000e 0029 0000 09 80a1 0016 68b10600 0003 007e040000201000020018000002c4000500

}
