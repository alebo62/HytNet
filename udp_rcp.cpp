#include "tcp.h"
#include "messages.h"
//#include <string>
#include "enums.h"
quint16 msg_cnt = 0;
quint32 state = 0;
unsigned char ack[] =    {0x7e,3,0,0x10,0x20,0x10,0,0,0,0x0C,0x60,0x00};
unsigned char conn[] =   {0x7e,4,0,0xfe,0x20,0x10,0,0,0,0x0c,0x60,0xe1};

extern hrnp_t         remote_monitor;
extern hrnp_t         dig_audio_tx_req;
extern hrnp_t         dig_audio_rx_req;
extern hrnp_t         rtp_pld_req;
extern hrnp_t         br_cast_req;
extern hrnp_t         radio_id;
extern hrnp_t         chan_status;
extern hrnp_t         serialNum_req;
extern hrnp_t         zone_ch;
extern hrnp_t         key_notific_req;
extern hrnp_t         txt_notific_req;
extern hrnp_t         bc_txt_notific_req;
extern hrnp_t         br_cast_analog;
extern hrnp_no_pld_t  pwr_up_chk_req;
extern hrnp_no_pld_t  ack_msg;
extern hrnp_no_pep_t  conn_req;
extern hrnp_no_pep_t  close_req;
extern hrnp_t 		  txt_notif_req;
extern hrnp_t		  txt_msg_tx;
extern hrnp_t		  uart_config_req;
extern hrnp_t         ptt_press_req;
extern strRMRpt 	  sRegMsgReport;
extern strARSmsg      sARSmsg;
extern QByteArray     tcp_tx;
extern strCRp        sCallReply;
extern strCRpt       sCallReport;
extern strCRp        sCallReply;
extern strCSRp       sCallStopReply;
extern strCtrlRpl    sCtrlReply;
extern strCCRpl      sChanChangeReply;
extern strCCRpt      sChanChangeReport;
extern strRldRpl     sReloadReplay;
extern strCtrlRpl    sCtrlReply;
extern quint8 req_id[];
extern void checksum(unsigned char*);
extern QString host;
extern unsigned char buf_tx[64];
extern quint32 Radio_Reg_State;
quint8 CallsDecoder[] = {3,1,5,2,4,6};
extern void receive_sound();
QByteArray ba_3005;
void TCP::rcv_udpRCP()
{
    ba_3005.resize(static_cast<int>(udpRCP_3005.pendingDatagramSize()));
    udpRCP_3005.readDatagram(ba_3005.data(), ba_3005.size());
    //qDebug() << "rcv RCP " << ba_3005.length();;
    if(Radio_Reg_State == INIT_STATE)
    {
        if(ba_3005.at(3) == 0xFD)
        {
            qDebug() << "FD";
            state = 1;
            rad_conn_tim.stop();
            ack[4] = pi_num = ba_3005.at(5);
            checksum(ack);
            //checksum(sbcr_1);
            msg_cnt++;
            pwr_up_chk_req.packet_num[0] = msg_cnt >> 8;// page 159
            pwr_up_chk_req.packet_num[1] = msg_cnt & 0xFF;
            memcpy(buf_tx , reinterpret_cast<char*>(&pwr_up_chk_req), pwr_up_chk_req.length[1]);
            checksum(buf_tx);
            udpRCP_3005.writeDatagram((char*)buf_tx, pwr_up_chk_req.length[1], QHostAddress(host), RCP);
            udpRCP_3005.flush();
        }
        else if((ba_3005.at(13) == 0xC6) && (ba_3005.at(14) == 0x80) && (state == 1))
        {
            state = 2;
            udpRCP_3005.writeDatagram((char*)ack, 12, QHostAddress(host), RCP);
            udpRCP_3005.flush();
            if(ba_3005.at(17) == 0)
            {
                qDebug() << "Radio_Reg_State : POWER UP check OK";
                msg_cnt++;
                zone_ch.packet_num[0] = msg_cnt >> 8;  // page 96
                zone_ch.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&zone_ch), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(zone_ch.pep.pld), zone_ch.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + zone_ch.pep.num_of_bytes[0], reinterpret_cast<char*>(&zone_ch + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, zone_ch.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
        }
        else if((ba_3005.at(13) == 0xC4)&& (ba_3005.at(14) == 0x80) && (state == 2))// && (rx[17] == 0))
        {
            state = 3;
            udpRCP_3005.writeDatagram((char*)ack, 12, QHostAddress(host), RCP);
            udpRCP_3005.flush();
            if((ba_3005.at(17) == 0) && (ba_3005.at(18) == 0) && (ba_3005.at(19) == 0) && (ba_3005.at(20) == 0))
            {
                qDebug() << "Zone Channel OK";
                //Radio_Reg_State = ZONE_CH;
                sRegMsgReport.zone[1] = ba_3005.at(25);
                sRegMsgReport.zone[0] = ba_3005.at(26);
                sRegMsgReport.channel[1] = ba_3005.at(27);
                sRegMsgReport.channel[0] = ba_3005.at(28);
                msg_cnt++;
                serialNum_req.packet_num[0] = msg_cnt >> 8;  // page 98
                serialNum_req.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&serialNum_req), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(serialNum_req.pep.pld), serialNum_req.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + serialNum_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&serialNum_req + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, serialNum_req.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            else
                qDebug() << "rx Zone Channel fail ";
        }
        else if((ba_3005.at(13) == 0x01)&& (ba_3005.at(14) == 0x82) && (state == 3))// && (rx[17] == 0))
        {
            state = 4;
            udpRCP_3005.writeDatagram((char*)ack, 12, QHostAddress(host), RCP);
            udpRCP_3005.flush();
            if((ba_3005.at(17) == 0) && (ba_3005.at(18) == 0) && (ba_3005.at(19) == 0) && (ba_3005.at(20) == 0))
            {
                qDebug() << "S/N OK";  //  8digits 45020005+00
                //Radio_Reg_State = SER_NUM;
                for(int i = 0; i < 8; i++)
                    sRegMsgReport.ser_num[9-i] = static_cast<unsigned char>(ba_3005.at(25 + (i << 1)));//  but len == 64
                sRegMsgReport.ser_num[0] = sRegMsgReport.ser_num[1] = 0x30;
                msg_cnt++;
                chan_status.packet_num[0] = msg_cnt >> 8; // page 103
                chan_status.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&chan_status), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(chan_status.pep.pld), chan_status.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + chan_status.pep.num_of_bytes[0], reinterpret_cast<char*>(&chan_status + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, chan_status.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            else
                qDebug() << "rx S/N msg fail " ;
        }
        else if((ba_3005.at(13) == 0xE7)&& (ba_3005.at(14) == 0x80) && (state == 4))// && (rx[17] == 0))
        {
            state = 5;
            udpRCP_3005.writeDatagram((char*)ack, 12, QHostAddress(host), RCP);
            udpRCP_3005.flush();
            if(ba_3005.at(17) == 0)
            {
                qDebug() << "Chan status OK";
                //       Radio_Reg_State = CH_STATUS;
                //0: conventional digital
                //1: conventional analog
                //2: repeater digital
                //3: repeater analog
                //4: trunking digital
                //5: trunking analog
                if(ba_3005.at(20) == 0)
                    sRegMsgReport.signal_mode = 1;
                else if(ba_3005.at(20) == 1)
                    sRegMsgReport.signal_mode = 0;
                else
                    qDebug() << "Unnown status:" << (int)ba_3005.at(20);

                msg_cnt++;
                radio_id.packet_num[0] = msg_cnt >> 8;  // page 139
                radio_id.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&radio_id), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(radio_id.pep.pld), radio_id.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + radio_id.pep.num_of_bytes[0], reinterpret_cast<char*>(&radio_id + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, radio_id.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            else
                qDebug() << "rx Chan Status msg fail ";
        }
        else if((ba_3005.at(13) == 0x52)&& (ba_3005.at(14) == 0x84) && (state == 5))
        {
            state = 6;
            udpRCP_3005.writeDatagram((char*)ack, 12, QHostAddress(host), RCP);
            udpRCP_3005.flush();
            if(ba_3005.at(17) == 0)
            {
                qDebug() << "Radio Id OK";

                sRegMsgReport.radio_id[3] = static_cast<unsigned char>(ba_3005.at(19));
                sRegMsgReport.radio_id[2] = static_cast<unsigned char>(ba_3005.at(20));
                sRegMsgReport.radio_id[1] = static_cast<unsigned char>(ba_3005.at(21));
                sRegMsgReport.radio_id[0] = static_cast<unsigned char>(ba_3005.at(22));
                sRegMsgReport.header.msgType = 1;
                sRegMsgReport.header.payloadLength[0] = 0;
                sRegMsgReport.header.payloadLength[1] = 24;
                sRegMsgReport.len[0] = 0;
                sRegMsgReport.len[1] = 10;
                sRegMsgReport.none = 0x00;
                sRegMsgReport.type = eRegReportMsg;
                sRegMsgReport.reg_unreg_state = REGISTRATE;

                tcp_conn_tim.start();

                // 01001800 01 00 000a 30303530303032303534 0001 0001 00 00000001 01
                // QByteArray ba;
                // ba.resize(sizeof(sRegMsgReport));
                // memcpy((char*)ba.data(), (char*)&sRegMsgReport, sizeof(sRegMsgReport));
                // qDebug() << ba.toHex();
                msg_cnt++;
                key_notific_req.packet_num[0] = msg_cnt >> 8;  // audio route rx tx  page232
                key_notific_req.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&key_notific_req), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(key_notific_req.pep.pld), key_notific_req.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + key_notific_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&key_notific_req + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, key_notific_req.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            else
                qDebug() << "rx Radio id fail " ;
        }
        else if((ba_3005.at(13) == 0xE4)&& (ba_3005.at(14) == 0x80) && (state == 6))
        {
            state = 7;
            udpRCP_3005.writeDatagram((char*)ack, 12, QHostAddress(host), RCP);
            udpRCP_3005.flush();
            if(ba_3005.at(17) == 0)
            {
                qDebug() << "Key Notification OK";
                msg_cnt++;
                //Radio_Reg_State = BRD_TXT_NOTIFIC;
                bc_txt_notific_req.packet_num[0] = msg_cnt >> 8;  // audio route rx tx  page232
                bc_txt_notific_req.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&bc_txt_notific_req), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(bc_txt_notific_req.pep.pld), bc_txt_notific_req.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + bc_txt_notific_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&bc_txt_notific_req + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, bc_txt_notific_req.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            else
                qDebug() << "Key Notification Fail";
        }
        else  if((ba_3005.at(13) == 0x47)&& (ba_3005.at(14) == 0x88) && (state == 7))
        {
            state = 8;
            udpRCP_3005.writeDatagram((char*)ack, 12, QHostAddress(host), RCP);
            udpRCP_3005.flush();
            if(ba_3005.at(17) == 0)
            {
                qDebug() << "BRD_TXT Notification OK";
                msg_cnt++;
                //Radio_Reg_State = TXT_NOTIFIC;
                rtp_pld_req.packet_num[0] = msg_cnt >> 8;  // audio route rx tx  page232
                rtp_pld_req.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&rtp_pld_req), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(rtp_pld_req.pep.pld), rtp_pld_req.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + rtp_pld_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&rtp_pld_req + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, rtp_pld_req.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            else
                qDebug() << "Key Notification Fail";
        }
        else if((ba_3005.at(13) == 0x19)&& (ba_3005.at(14) == 0x84) && (state == 8))
        {
            state = 9;
            udpRCP_3005.writeDatagram((char*)ack, 12, QHostAddress(host), RCP);
            udpRCP_3005.flush();
            if(ba_3005.at(17) == 0x00)
            {
                qDebug() << "RTP Success";
                msg_cnt++;
                //Radio_Reg_State = ROUTE_TX;
                dig_audio_rx_req.packet_num[0] = msg_cnt >> 8;  // audio route rx tx  page232
                dig_audio_rx_req.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&dig_audio_rx_req), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(dig_audio_rx_req.pep.pld), dig_audio_rx_req.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + dig_audio_rx_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&dig_audio_rx_req + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, dig_audio_rx_req.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            else
                qDebug() << " RTP Fail";
        }
        else if((ba_3005.at(13) == 0xDF)&& (ba_3005.at(14) == 0x80) && (state == 9))
        {
            state = 10;
            udpRCP_3005.writeDatagram((char*)ack, 12, QHostAddress(host), RCP);
            udpRCP_3005.flush();
            if(ba_3005.at(17) == 0x00)
            {
                qDebug() << "Rx route Success";
                msg_cnt++;
                dig_audio_tx_req.packet_num[0] = msg_cnt >> 8;  // audio route rx tx  page232
                dig_audio_tx_req.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&dig_audio_tx_req), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(dig_audio_tx_req.pep.pld), dig_audio_tx_req.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + dig_audio_tx_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&dig_audio_tx_req + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, dig_audio_tx_req.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            else
            {
                qDebug() << "Rx route fail";
            }
        }
        else if((ba_3005.at(13) == 0xDF)&& (ba_3005.at(14) == 0x80) && (state == 10))
        {
            state = 11;
            udpRCP_3005.writeDatagram((char*)ack, 12, QHostAddress(host), RCP);
            udpRCP_3005.flush();
            if(ba_3005.at(17) == 0x00)
            {
                qDebug() << "Tx route Success";
                msg_cnt++;
                br_cast_req.packet_num[0] = msg_cnt >> 8; // broadcast rx and tx  page80
                br_cast_req.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&br_cast_req), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(br_cast_req.pep.pld), br_cast_req.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + br_cast_req.pep.num_of_bytes[0], reinterpret_cast<char*>(&br_cast_req + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, br_cast_req.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            else
                qDebug() << "Tx route Fail";
        }
        else if((ba_3005.at(13) == 0xC9)&& (ba_3005.at(14) == 0x80) && (state == 11))
        {
            Radio_Reg_State = READY; // End INitialization !!!!!!!!!!!
            udpRCP_3005.writeDatagram((char*)ack, 12, QHostAddress(host), RCP);
            udpRCP_3005.flush();
            if(ba_3005.at(17) == 0x00)
            {
                qDebug() << "BroadCast Success";
            }
            else {
                qDebug() << "BroadCast Fail";
            }
        }
    }
    else
    {
        switch (Radio_Reg_State)
        {
        case WAIT_CALL_REPLY:
            if((ba_3005.at(13) == 0x41) && (ba_3005.at(14) == 0x88))
            {
                if(ba_3005.at(17) == 0)
                {
                    sCallReply.requestResult = eSucces;
                    Radio_Reg_State = WAIT_CALL_REPORT;
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
                else
                {
                    sCallReply.requestResult = eFailure;
                    Radio_Reg_State = READY;// a если был прием???
                    tcp_srv.write(reinterpret_cast<char*>(&sCallReply), sizeof(sCallReply));
                }
            }
            break;
        case WAIT_CALL_REPORT:
            if((ba_3005.at(13) == 0x41) && (ba_3005.at(14) == 0x80)) // page 94
            {
                if(ba_3005.at(17) == 0)
                {
                    //                    memcpy( sCallReport.calledId, sRegMsgReport.radio_id , 4);// its my id
                    //                    sCallReport.callState = eCallInit;
                    //                    tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
                }
                else
                {
                    Radio_Reg_State = READY;
                    sCallReport.callState = eCallEnded;
                    memcpy( sCallReport.calledId, sRegMsgReport.radio_id , 4);// its my id
                    tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
                }
            }
            else if((ba_3005.at(13) == 0x43) && (ba_3005.at(14) == 0xB8))
            {
                if(ba_3005.at(17) == 1)// voice call
                {
                    Radio_Reg_State = WAIT_STOP_CALL_REPLY;
                    memcpy( sCallReport.calledId, sRegMsgReport.radio_id , 4);// its my id
                    sCallReport.callState = eCallInit;
                    tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
                }
                else
                {
                    Radio_Reg_State = READY;
                    sCallReport.callState = eCallEnded;
                    memcpy( sCallReport.calledId, sRegMsgReport.radio_id , 4);// its my id
                    tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
                }
            }
            break;
            // ********************************** tx ****************************************
        case WAIT_STOP_CALL_REPLY:
            if((ba_3005.at(13) == 0x41) && (ba_3005.at(14) == 0x80)) // page 94
            {
                if(ba_3005.at(17) == 0)
                {
                    Radio_Reg_State = WAIT_STOP_CALL_HANGIN;
                    sCallStopReply.replyResult = eSucces;
                    tcp_srv.write(reinterpret_cast<char*>(&sCallStopReply), sizeof(sCallStopReply));
                    qDebug() << "WAIT_STOP_CALL_REPLY";
                }
                else
                {
                    Radio_Reg_State = READY;
                }
            }
            break;
        case WAIT_STOP_CALL_HANGIN:
            if((ba_3005.at(13) == 0x43) && (ba_3005.at(14) == 0xB8)) // page 82
            {
                if(ba_3005.at(17) == 2)
                {
                    sCallReport.callType = CallsDecoder[ba_3005.at(21)];
                    sCallReport.callState = eCallHandTime;
                    memcpy( &sCallReport.calledId, &sRegMsgReport.radio_id , 4);
                    sCallReport.receivedId[3] = ba_3005.at(23);
                    sCallReport.receivedId[2] = ba_3005.at(24);
                    sCallReport.receivedId[1] = ba_3005.at(25);
                    tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
                    Radio_Reg_State = WAIT_STOP_CALL_ENDED;
                    qDebug() << "WAIT_STOP_CALL_HANGIN";
                }
                else
                {

                }
            }

            break;
        case WAIT_STOP_CALL_ENDED:
            if((ba_3005.at(13) == 0x43) && (ba_3005.at(14) == 0xB8)) // page 82
            {
                if(ba_3005.at(17) == 3)
                {
                    sCallReport.callType = CallsDecoder[ba_3005.at(21)];
                    sCallReport.callState = eCallEnded;
                    memcpy( sCallReport.calledId, sRegMsgReport.radio_id , 4);
                    sCallReport.receivedId[3] = ba_3005.at(23);
                    sCallReport.receivedId[2] = ba_3005.at(24);
                    sCallReport.receivedId[1] = ba_3005.at(25);
                    tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
                    Radio_Reg_State = READY;
                    qDebug() << "WAIT_STOP_CALL_ENDED";
                }
                else
                {

                }
            }
            //            else if((buf_rx[13] == 0x44)&& (buf_rx[14] == 0xB8)) // input call
            //                    {
            //                        if(buf_rx[17] == 1)
            //                        {
            //                            sCallReport.callType = CallsDecoder[buf_rx[19]];
            //                            sCallReport.callState = eCallInit;
            //                            sCallReport.receivedId[1] = buf_rx[23];
            //                            sCallReport.receivedId[2] = buf_rx[22];
            //                            sCallReport.receivedId[3] = buf_rx[21];

            //                            sCallReport.calledId[1] = buf_rx[27];
            //                            sCallReport.calledId[2] = buf_rx[26];
            //                            sCallReport.calledId[3] = buf_rx[25];
            //                            kupol.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
            //            #ifdef DBG
            //                            qDebug() << "voice rx";
            //            #endif
            //                            Radio_Reg_State = RX;
            //                        }
            //                    }
            break;
        case PTT_PRESS_AN:
            qDebug() << "PTT Press ";
            memcpy( sCallReport.calledId, sRegMsgReport.radio_id , 4);// its my id
            memset(sCallReport.receivedId, 0, 4);
            sCallReport.callType = 1;
            sCallReport.callState = eCallInit;
            tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
            break;
        case PTT_RELEASE_AN:
            sCallReport.callType = 1;
            sCallReport.callState = eCallEnded;
            memcpy( sCallReport.calledId, sRegMsgReport.radio_id , 4);
            memset(sCallReport.receivedId, 0, 4);
            tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
            Radio_Reg_State = READY;
            qDebug() << "WAIT_STOP_CALL_ENDED";
            break;
        case WAIT_CTRL_REPLY:
            if((ba_3005.at(13) == 0x34) && (ba_3005.at(14) == 0x88)) // page 82
            {
                sCtrlReply.radio_id[0] = ba_3005.at(21);
                sCtrlReply.radio_id[1] = ba_3005.at(20);
                sCtrlReply.radio_id[2] = ba_3005.at(19);
                sCtrlReply.radio_id[3] = ba_3005.at(18);
                if(ba_3005.at(13) == 0x00)
                {
                    sCtrlReply.result = eSucces;
                }
                else
                {
                    sCtrlReply.result = eFailure;
                }
                tcp_srv.write(reinterpret_cast<char*>(&sCtrlReply), sizeof(sCtrlReply));
            }
            break;
        case WAIT_RELOAD_REPLY:
            if((ba_3005.at(13) == 0xC1) && (ba_3005.at(14) == 0x81)) // page 160
            {
                sReloadReplay.result = static_cast<unsigned char>(ba_3005.at(17));
                tcp_srv.write(reinterpret_cast<char*>(&sReloadReplay), sizeof(sReloadReplay));

                if(sReloadReplay.result == eSucces)
                {
                    Radio_Reg_State = INIT_STATE;
                    sRegMsgReport.reg_unreg_state = UNREGISTRATE;
                    tcp_srv.write(reinterpret_cast<char*>(&sRegMsgReport), sizeof(sRegMsgReport));
                    reload_tim.start();
                    tcp_srv.disconnectFromHost();
                }
                else
                {
                    Radio_Reg_State = READY;
                }
            }
            break;
        case READY:

            if((ba_3005.at(13) == 0x44) && (ba_3005.at(14) == 0xB8)) // input call
            {
                if(ba_3005.at(17)   == 1)
                {
                    sCallReport.callType = CallsDecoder[ba_3005.at(19)];
                    sCallReport.callState = eCallInit;
                    sCallReport.receivedId[1] = ba_3005.at(23);
                    sCallReport.receivedId[2] = ba_3005.at(22);
                    sCallReport.receivedId[3] = ba_3005.at(21);

                    sCallReport.calledId[1] = ba_3005.at(27);
                    sCallReport.calledId[2] = ba_3005.at(26);
                    sCallReport.calledId[3] = ba_3005.at(25);
                    tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
                    tcp_srv.flush();
#ifdef DBG
                    qDebug() << "voice rx";
#endif
                    Radio_Reg_State = RX;
                }
            }
        case RX:
            if(ba_3005.at(3) == VOICE)// page85
            {
                qDebug() << "voice";
                receive_sound();
            }
            else if((ba_3005.at(3) == DATA) && (ba_3005.at(13) == 0x44)&& (ba_3005.at(14) == 0xB8)) // input call HANG_TIME
            {
                if(ba_3005.at(17) == 2)
                {
                    //sCallReport.callType = CallsDecoder[buf_rx[19]];
                    sCallReport.callState = eCallHandTime;
                    //                sCallReport.receivedId[1] = buf_rx[23];
                    //                sCallReport.receivedId[2] = buf_rx[22];
                    //                sCallReport.receivedId[3] = buf_rx[21];

                    //                sCallReport.calledId[1] = buf_rx[27];
                    //                sCallReport.calledId[2] = buf_rx[26];
                    //                sCallReport.calledId[3] = buf_rx[25];
                    tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
                    tcp_srv.flush();
#ifdef DBG
                    qDebug() << "hang time";
#endif
                    Radio_Reg_State = RX_HT;
                }
            }

            break;
        case RX_HT:
            if(ba_3005.at(3) == VOICE)// page85
            {
#ifdef DBG
                qDebug() << "hang time sound";
#endif
                sCallReport.callState = eCallInit;
                tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
                tcp_srv.flush();
                Radio_Reg_State = RX;
                //receive_Sound();
            }
            else if((ba_3005.at(13) == 0x44)&& (ba_3005.at(14) == 0xB8)) // input call
            {
                if(ba_3005.at(17) == 3)
                {
                    //sCallReport.callType = CallsDecoder[buf_rx[19]];

                    sCallReport.callState = eCallEnded;
                    //                sCallReport.receivedId[1] = buf_rx[23];
                    //                sCallReport.receivedId[2] = buf_rx[22];
                    //                sCallReport.receivedId[3] = buf_rx[21];

                    //                sCallReport.calledId[1] = buf_rx[27];
                    //                sCallReport.calledId[2] = buf_rx[26];
                    //                sCallReport.calledId[3] = buf_rx[25];
                    tcp_srv.write(reinterpret_cast<char*>(&sCallReport), sizeof(sCallReport));
                    tcp_srv.flush();
#ifdef DBG
                    qDebug() << "call end";
#endif
                    Radio_Reg_State = READY;
                }
            }
            break;
        }
    }
}
