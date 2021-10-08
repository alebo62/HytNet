#include "tcp.h"
#include "messages.h"
//#include <string>
#include "enums.h"
//quint16 msg_cnt = 0;
//quint32 state = 0;

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


extern quint8 req_id[];
extern void checksum(unsigned char*);
extern QString host;





QByteArray ba_3003;
void TCP::rcv_udpGPS()
{
    ba_3003.resize(static_cast<int>(udpGPS_3003.pendingDatagramSize()));
    udpGPS_3003.readDatagram(ba_3003.data(), ba_3003.size());
}
QByteArray ba_3004;
void TCP::rcv_udpTMS()
{
    ba_3004.resize(static_cast<int>(udpTMS_3004.pendingDatagramSize()));
    udpTMS_3004.readDatagram(ba_3004.data(), ba_3004.size());
}


QByteArray ba_3006;
void TCP::rcv_udpTLM()
{
    ba_3006.resize(static_cast<int>(udpTLM_3006.pendingDatagramSize()));
    udpTLM_3006.readDatagram(ba_3006.data(), ba_3006.size());

}
QByteArray ba_3007;
void TCP::rcv_udpDTR()
{
    ba_3007.resize(static_cast<int>(udpDTR_3007.pendingDatagramSize()));
    udpDTR_3007.readDatagram(ba_3007.data(), ba_3007.size());

}
QByteArray ba_3009;
void TCP::rcv_udpSDM()
{
    ba_3009.resize(static_cast<int>(udpSDM_3009.pendingDatagramSize()));
    udpSDM_3009.readDatagram(ba_3009.data(), ba_3009.size());
}
