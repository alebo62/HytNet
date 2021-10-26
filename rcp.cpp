#include "tcp.h"
#include "messages.h"
#include "enums.h"

extern void checksum(unsigned char*);
extern void crc(quint8*, int);

extern QString          ars_addr;
extern quint8           ars_answer[16];
extern quint8           ars_check[11];
extern QByteArray       ba;
extern unsigned char    buf_tx[];
extern hrnp_t           call_req;
extern bool             conn_state_kupol;
extern QString          dest_rrs_ip;
extern strMsgHeader     header;
extern QString          hadrr, host;
extern quint16          hyt_udp_port;
extern quint16          hyterus_port;
extern quint8           last_abonent[4];
extern volatile quint16 msg_cnt;
extern quint8           mu_law;
extern quint32          ping_counter;
extern hrnp_t           ptt_release_req;
extern hrnp_t           ptt_press_req;
extern hrnp_t           radio_en_dis;
extern quint32          Radio_Reg_State;
extern quint8           req_id[4];
extern hrnp_t           restart;
extern hrnp_t           remote_monitor;
extern rtp_header       rtp_hdr;
extern hrnp_t           rtp_pld_req;
extern strMRRpt         sMRRpt;
extern strCtrlRpl       sCtrlReply;
extern quint32          Radio_Reg_State;
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
extern QByteArray       tcp_tx;
extern hrnp_t           zone_ch;
extern hrnp_t           radio_check;

quint32 check_online = 0;
quint32 disable_online = 0;
quint32 enable_online = 0;
quint32 monitor_online = 0;

void TCP::rcv_tcpRCP()
{
    if(ba.at(4) == eCallRequestMsg)//тип(4)+ резерв(5)+id_req(6..9)+calltype(10)+rcvId(11..14)
    {
        if(sRegMsgReport.signal_mode == DIGITAL_MODE)
        {
            if(Radio_Reg_State == READY)// init call
            {
                memcpy(sCallReply.reqid, ba.data() + 6, 4);
                sCallReport.callType = ba.at(10);
                if(ba.at(10) == eGroupCall)
                {
                    memcpy((char*)&sCallReport.receivedId[1], ba.data() + 4 + 4 +  sizeof (header), 3);
                    call_req.pep.pld[0] = eGroupCall; // call type
                    call_req.pep.pld[1] = ba.at(14);
                    call_req.pep.pld[2] = ba.at(13);
                    call_req.pep.pld[3] = ba.at(12);
                    call_req.pep.pld[4] = 0;
                    memcpy(last_abonent, ba.data() + 12, 3);
                }
                else if(ba.at(10) == ePrivCall)
                {
                    call_req.pep.pld[0] = 0;
                    memcpy((char*)&sCallReport.receivedId[1], ba.data() + 4 + 4 +  sizeof (header), 3);
                    call_req.pep.pld[1] = ba.at(14);
                    call_req.pep.pld[2] = ba.at(13);
                    call_req.pep.pld[3] = ba.at(12);
                    call_req.pep.pld[4] = 0;
                    memcpy(last_abonent, ba.data()  + 4 + 4 +  sizeof (header), 3);
                }
                else if(ba.at(10) == eAllCall)
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
                tx_tim.start();
            }
            else if (Radio_Reg_State == RX_HT) // quick answer (only press ptt)
            {
                memcpy(sCallReply.reqid, ba.data() + 6, 4);
                sCallReport.callType = ba.at(10);
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
                tx_tim.start();
                rx_tim.stop();
            }
        }
        else if(sRegMsgReport.signal_mode == ANALOG_MODE)
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
                Radio_Reg_State = WAIT_STOP_CALL_REPLY;
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
                //udpRCP_3005.flush();
                qDebug() << "send ptt release";
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
    else if(ba.at(4) == eSuControlRequest)////тип(4)+ резерв(5)+id_req(6..9)+ReqType(10)+rcvId(11..14)
    {
        memcpy(sCtrlReply.reqid, ba.data() + 6, 4);
        memcpy(sCtrlReply.radio_id, ba.data() + 11, 4);
        switch (ba.at(10)) {
        case 0: // Radio Check
            if(!check_online)
            {
                check_online = 1;
                radio_check_tim.start();
                memcpy(sCtrlReply.radio_id, ba.data() + 11, 4);
                sCtrlReply.req_type = 0; // radio check
                ars_addr = dest_rrs_ip + QString::number(ba.at(12))+ "."+QString::number(ba.at(13))+"."+ QString::number(ba.at(14));
                ars_check[6] = ba.at(12);
                ars_check[7] = ba.at(13);
                ars_check[8] = ba.at(14);
                crc(ars_check, 11);
                udpRRS_3002.writeDatagram((char*)ars_check, 11, QHostAddress(ars_addr), RRS);
                udpRRS_3002.flush();
            }
            //            qDebug() << "radio check";
            break;
        case 1:// Inhibid
            if(!disable_online)
            {
                disable_online = 1;
                radio_en_dis.pep.opcode[0] = 0x49;
                radio_en_dis.pep.pld[0] = ba.at(14);
                radio_en_dis.pep.pld[1] = ba.at(13);
                radio_en_dis.pep.pld[2] = ba.at(12);
                msg_cnt++;
                radio_en_dis.packet_num[0] = msg_cnt >> 8;
                radio_en_dis.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&radio_en_dis), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(radio_en_dis.pep.pld), radio_en_dis.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + radio_en_dis.pep.num_of_bytes[0], reinterpret_cast<char*>(&radio_en_dis + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, radio_en_dis.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            break;
        case 2: // Uninhibid
            if(!enable_online)
            {
                enable_online = 1;
                radio_en_dis.pep.opcode[0] = 0x4A;
                radio_en_dis.pep.pld[0] = ba.at(14);
                radio_en_dis.pep.pld[1] = ba.at(13);
                radio_en_dis.pep.pld[2] = ba.at(12);
                msg_cnt++;
                radio_en_dis.packet_num[0] = msg_cnt >> 8;
                radio_en_dis.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&radio_en_dis), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(radio_en_dis.pep.pld), radio_en_dis.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + radio_en_dis.pep.num_of_bytes[0], reinterpret_cast<char*>(&radio_en_dis + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, radio_en_dis.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            break;
        case 3://  Monitor
            if(!monitor_online)
            {
                monitor_tim.start();
                monitor_online = 1;
                remote_monitor.pep.pld[0] = ba.at(14);
                remote_monitor.pep.pld[1] = ba.at(13);
                remote_monitor.pep.pld[2] = ba.at(12);
                msg_cnt++;
                remote_monitor.packet_num[0] = msg_cnt >> 8;
                remote_monitor.packet_num[1] = msg_cnt & 0xFF;
                memcpy(buf_tx, reinterpret_cast<char*>(&remote_monitor), 17 );
                memcpy(buf_tx + 17, reinterpret_cast<char*>(remote_monitor.pep.pld), remote_monitor.pep.num_of_bytes[0]);
                memcpy(buf_tx + 17 + remote_monitor.pep.num_of_bytes[0], reinterpret_cast<char*>(&remote_monitor + 21), 2 );
                checksum(buf_tx);
                udpRCP_3005.writeDatagram((char*)buf_tx, remote_monitor.length[1], QHostAddress(host), RCP);
                udpRCP_3005.flush();
            }
            break;
        case 4:// Dekey

            break;
        default:
            break;
        }
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
        hyt_udp_port = static_cast<quint16>(static_cast<quint32>( ba.at(8 + sizeof(header))) * 256 + static_cast<quint32>(ba.at(9 + sizeof(header))));
        udp_srv.bind(hyt_udp_port);
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
}
