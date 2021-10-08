#include "tcp.h"
#include "messages.h"
#include "enums.h"
#include "/home/alex/raspi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/arm-linux-gnueabihf/libc/usr/include/string.h"

using namespace std;

extern void checksum(unsigned char*);
extern void crc(quint8*, int);

extern QString hadrr, host;
extern quint16 hyterus_port;
extern unsigned char buf_tx[];
extern quint16 msg_cnt;

extern strMRRpt                 sMRRpt;
extern strCtrlRpl               sCtrlReply;
extern strRldRpl                sReloadReplay;
extern strCCRpt                 sChanChangeReport;
extern strCCRpl                 sChanChangeReply;
extern strCRpt                  sCallReport;
extern strCSRp                  sCallStopReply;
extern strCSRp                  sStopCallReply;
extern strCRp                   sCallReply;
extern strCallControl           sCallControl;
extern strRMRpt                 sRegMsgReport;
extern struct strMsgHeader      header;
extern struct strMsgType_Ping   sMsgType_Ping;
//extern quint16          packet_num;
extern rtp_header       rtp_hdr;
extern quint32          Radio_Reg_State;
extern hrnp_t           rtp_pld_req;
extern hrnp_t           call_req;
extern hrnp_t           ptt_release_req;
extern hrnp_t           zone_ch;
extern hrnp_t           ptt_press_req;
extern hrnp_t           restart;
extern hrnp_t           remote_monitor;
extern hrnp_t 			txt_msg_tx;

extern quint8           ars_answer[16];
extern quint8           ars_check[11];
extern QString          dest_rrs_ip;
//QString                 ars_addr;

quint32     Radio_Reg_State;
quint32     ping_counter;
quint8      last_abonent[4];
quint8      mu_law;
QByteArray  tcp_tx;
QByteArray  ba;
bool        conn_state_kupol;
quint8     req_id[4];

void TCP::tcp_receive()
{
    ba = tcp_srv.read(256);
    ping_tim.start();
    qDebug() << "tcp:" << ba.toHex();
    switch (ba.at(0)) {
    case 4: // ping message
        qDebug() << "ping send";
        tcp_srv.write(reinterpret_cast<char*>(&sMsgType_Ping), sizeof(sMsgType_Ping));
        break;
    case 1: // control messages
        if(ba.at(sizeof(header)) == eCallRequestMsg)
        {
            //audio_buff_timer = 0;
            if(sRegMsgReport.signal_mode) // digital mode
            {
                if(Radio_Reg_State == READY)
                {
                    memcpy(sCallReply.reqid, ba.data() + 2 + sizeof(header), 4);
                    //sCallControl.func = eInitiate;
                    //sCallControl.calltype= HyteraCallTypes[ba.at(2 + 4 + sizeof (header))];
                    sCallReport.callType = ba.at(2 + 4 + sizeof (header));
                    if(ba.at(2 + 4 + sizeof (header)) == eGroupCall)
                    {
                        call_req.pep.pld[0] = 1; // call type
                        memcpy((char*)&sCallReport.receivedId[1], ba.data() + 4 + 4 +  sizeof (header), 3);
                        call_req.pep.pld[1] = ba.at(14);
                        call_req.pep.pld[2] = ba.at(13);
                        call_req.pep.pld[3] = ba.at(12);
                        call_req.pep.pld[4] = 0;
                        //memcpy((char*)&call_req.pep.pld[1], ba.data() + 4 + 4 +  sizeof (header) , 1);
                        //qDebug() << hex << call_req.pep.pld[1] << call_req.pep.pld[2] << call_req.pep.pld[3];
                        memcpy(last_abonent, ba.data()  + 4 + 4 +  sizeof (header), 3);
                    }
                    else if(ba.at(2 + 4 + sizeof (header)) == ePrivCall)
                    {
                        call_req.pep.pld[0] = 0;
                        memcpy((char*)&sCallReport.receivedId[1], ba.data() + 4 + 4 +  sizeof (header), 3);
                        call_req.pep.pld[1] = ba.at(14);
                        call_req.pep.pld[2] = ba.at(13);
                        call_req.pep.pld[3] = ba.at(12);
                        call_req.pep.pld[4] = 0;
                        memcpy(last_abonent, ba.data()  + 4 + 4 +  sizeof (header), 3);
                    }
                    else if(ba.at(2 + 4 + sizeof (header)) == eAllCall)
                    {
                        call_req.pep.pld[0] = 2;
                        memcpy((char*)&sCallReport.receivedId[1], ba.data() + 4 + 4 +  sizeof (header), 3);
                        memcpy(last_abonent, ba.data()  + 4 + 4 +  sizeof (header), 3);
                    }
                    msg_cnt++;
                    call_req.packet_num[0] = msg_cnt >> 8; // page 230  pcm pmu  pma  0 8 0x78
                    call_req.packet_num[1] = msg_cnt & 0xFF;
                    memcpy(buf_tx, reinterpret_cast<char*>(&call_req), 17 );
                    memcpy(buf_tx + 17, reinterpret_cast<char*>(call_req.pep.pld), call_req.pep.num_of_bytes[0]);
                    memcpy(buf_tx + 17 + call_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&call_req + 21), 2 );
                    checksum(buf_tx);
                    Radio_Reg_State = WAIT_CALL_REPLY;
                    udpRCP_3005.writeDatagram((char*)buf_tx, call_req.length[1], QHostAddress(host), RCP);
                    udpRCP_3005.flush();
                }
            }
            else // analog mode
            {
                memcpy(sCallReply.reqid, ba.data() + 2 + sizeof(header), 4);
                sCallReply.requestResult = eSucces;
                Radio_Reg_State = PTT_PRESS_AN;
                tcp_srv.write(reinterpret_cast<char*>(&sCallReply), sizeof(sCallReply));
                msg_cnt++;
                ptt_press_req.packet_num[0] = msg_cnt >> 8;// page 159
                ptt_press_req.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&ptt_press_req), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(ptt_press_req.pep.pld), ptt_press_req.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + ptt_press_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&ptt_press_req + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, ptt_press_req.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
        }
        else if(ba.at(sizeof(header)) == eStopCallRequestMsg)
        {
            if(sRegMsgReport.signal_mode) // digital mode
            {
                //sound_timer.stop();
                if((Radio_Reg_State == WAIT_STOP_CALL_REPLY) || (Radio_Reg_State == WAIT_STOP_CALL_HANGIN))// 0x0A
                {
                    //sound1_timer.stop();
                    //Radio_Reg_State = WAIT_STOP_CALL_REPLY;
                    //start_sound = 0;
                    //sound_buff_cnt_in = 0;
                    memcpy(sCallStopReply.reqid, ba.data() + 2 + sizeof(header), 4);
                    msg_cnt++;
                    ptt_release_req.packet_num[0] = msg_cnt >> 8;// page 159
                    ptt_release_req.packet_num[1] = msg_cnt & 0xFF;
                    memcpy(buf_tx, reinterpret_cast<char*>(&ptt_release_req), 17 );
                    memcpy(buf_tx + 17, reinterpret_cast<char*>(ptt_release_req.pep.pld), ptt_release_req.pep.num_of_bytes[0]);
                    memcpy(buf_tx + 17 + ptt_release_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&ptt_release_req + 21), 2 );
                    checksum(buf_tx);
                    //returned = libusb_bulk_transfer(usb_hdl, 4, buf_tx, ptt_release_req.length[1], &act_len, 1000);
                    udpRCP_3005.writeDatagram((char*)buf_tx, ptt_release_req.length[1], QHostAddress(host), RCP);
                    udpRCP_3005.flush();
                }
            }
            else // analog mode
            {
                memcpy(sCallStopReply.reqid, ba.data() + 2 + sizeof(header), 4);
                sCallStopReply.replyResult = eSucces;
                tcp_srv.write(reinterpret_cast<char*>(&sCallStopReply), sizeof(sCallStopReply));
                qDebug() << "WAIT_STOP_CALL_REPLY";
                Radio_Reg_State = PTT_RELEASE_AN;
                msg_cnt++;
                ptt_release_req.packet_num[0] = msg_cnt >> 8;// page 159
                ptt_release_req.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&ptt_release_req), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(ptt_release_req.pep.pld), ptt_release_req.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + ptt_release_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&ptt_release_req + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, ptt_release_req.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
        }
        if (ba.at(sizeof(header)) == eChanChangeRequestMsg)
        {
            memcpy(sChanChangeReply.reqid, ba.data() + 2 + sizeof(header), 4);
            zone_ch.pep.pld[2] = ba.at( 6 + sizeof(header));
            zone_ch.pep.pld[1] = ba.at( 6 + sizeof(header) + 1);
            zone_ch.pep.pld[4] = ba.at( 8 + sizeof(header));
            zone_ch.pep.pld[3] = ba.at( 8 + sizeof(header) + 1);
            msg_cnt++;
            zone_ch.pep.pld[0] = 0; // set
            zone_ch.packet_num[0] = msg_cnt >> 8;  // page 96
            zone_ch.packet_num[1] = msg_cnt & 0xFF;
            memcpy(buf_tx, reinterpret_cast<char*>(&zone_ch), 17 );
            memcpy(buf_tx + 17, reinterpret_cast<char*>(zone_ch.pep.pld), zone_ch.pep.num_of_bytes[0]);
            memcpy(buf_tx + 17 + zone_ch.pep.num_of_bytes[0], reinterpret_cast<char*>(&zone_ch + 21), 2 );
            checksum(buf_tx);
            udpRCP_3005.writeDatagram((char*)buf_tx, zone_ch.length[1], QHostAddress(host), RCP);
            udpRCP_3005.flush();
            qDebug() << "transmit Zone_ch";
            Radio_Reg_State = WAIT_CH_CHANGE_REPLY;
        }
        else if(ba.at(sizeof(header)) == eSuControlRequest)
        {
//            memcpy(sCtrlReply.reqid, ba.data() + 2 + sizeof(header), 4);
//            sCtrlReply.req_type = static_cast<unsigned char>(ba.at(sizeof(header) + 6));
//            remote_monitor.pep.pld[0] = ba.at(10 + sizeof(header));
//            remote_monitor.pep.pld[1] = ba.at(9  + sizeof(header));
//            remote_monitor.pep.pld[2] = ba.at(8  + sizeof(header));
//            remote_monitor.pep.pld[3] = ba.at(7  + sizeof(header));
//            msg_cnt++;
//            remote_monitor.packet_num[0] = msg_cnt >> 8;  // page 96
//            remote_monitor.packet_num[1] = msg_cnt & 0xFF;
//            remote_monitor.pep.pld[0] = 0; // set
//            memcpy(buf_tx, reinterpret_cast<char*>(&remote_monitor), 17 );
//            memcpy(buf_tx + 17, reinterpret_cast<char*>(remote_monitor.pep.pld), remote_monitor.pep.num_of_bytes[0]);
//            memcpy(buf_tx + 17 + remote_monitor.pep.num_of_bytes[0], reinterpret_cast<char*>(&remote_monitor + 21), 2 );
//            checksum(buf_tx);
//            udpRCP_3005.writeDatagram((char*)buf_tx, remote_monitor.length[1], QHostAddress(host), RCP);
//            udpRCP_3005.flush();
//            qDebug() << "transmit remote_monitor" ;
//            Radio_Reg_State = WAIT_CTRL_REPLY;
        }
        else if(ba.at(sizeof(header)) == eUdpPortNotification)
        {
            hyterus_port = static_cast<quint16>(static_cast<quint32>( ba.at(8 + sizeof(header))) * 256 + static_cast<quint32>(ba.at(9 + sizeof(header))));
            udp_srv.bind(hyterus_port);
#ifdef RTP
            mu_law = (ba.at(13 + sizeof(header)));
            rtp_hdr.payload_type = mu_law;
#endif
            connect(&udp_srv, SIGNAL(readyRead()),this, SLOT(udp_srv_slot()));

#ifdef DBG
            qDebug() << QDateTime::currentDateTime().toString(" yyyy-MM-dd hh:mm:ss ") << "port number: " << hyterus_port;
#endif
            qDebug() << "mulaw:" << mu_law;
            conn_state_kupol = true;

            //            switch (mu_law) {
            //            case 0:
            //                rtp_pld_req.pep.pld[0] = 8; // muLaw
            //                break;
            //            case 8:
            //                rtp_pld_req.pep.pld[0] = 0; // alow
            //                break;
            //            case 96:
            //rtp_pld_req.pep.pld[0] = 0x78; // PCM
            //                break;
            //            default:
            //                break;
            //            }
            //            msg_cnt++;
            //            rtp_pld_req.packet_num[0] = msg_cnt >> 8; // page 230  pcm pmu  pma  0 8 0x78
            //            rtp_pld_req.packet_num[1] = msg_cnt & 0xFF;
            //            memcpy(buf_tx, reinterpret_cast<char*>(&rtp_pld_req), 17 );
            //            memcpy(buf_tx + 17, reinterpret_cast<char*>(rtp_pld_req.pep.pld), rtp_pld_req.pep.num_of_bytes[0]);
            //            memcpy(buf_tx + 17 + rtp_pld_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&rtp_pld_req + 21), 2 );
            //            checksum(buf_tx);
            //            Radio_Reg_State = INIT_STATE;
            //            udpRCP_3005.writeDatagram((char*)buf_tx, rtp_pld_req.length[1], QHostAddress(host), RCP);
            //            udpRCP_3005.flush();
        }
        else if(ba.at(sizeof(header)) == eRldRequestMsg)
        {
            qDebug() << "reload";
            //            memcpy(sReloadReplay.reqid, ba.data() + 2 + sizeof(header), 4);
            //            Radio_Reg_State = WAIT_RELOAD_REPLY;
            //            msg_cnt++;
            //            restart.packet_num[0] = msg_cnt >> 8; // page 230  pcm pmu  pma  0 8 0x78
            //            restart.packet_num[1] = msg_cnt & 0xFF;
            //            memcpy(buf_tx, reinterpret_cast<char*>(&restart), 17 );
            //            //memcpy(buf_tx + 17, reinterpret_cast<char*>(restart.pep.pld), restart.pep.num_of_bytes[0]);
            //            memcpy(buf_tx + 17 + restart.pep.num_of_bytes[0], reinterpret_cast<char*>(&restart + 21), 2 );
            //            checksum(buf_tx);
            //            udpRCP_3005.writeDatagram((char*)buf_tx, restart.length[1], QHostAddress(host), RCP);
            //            udpRCP_3005.flush();
        }
        break;
    case 3: // location messages
        if(Radio_Reg_State == READY)
        {
            qDebug() << "gps";
        }
        break;
    case 0: // ars messages
        if(Radio_Reg_State == READY)
        {
            //qDebug() << "tcp1_ars " << ba.toHex();
            rcv_tcpRRS();
        }
        break;
    case 2: // text messages
        if(Radio_Reg_State == READY)
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


