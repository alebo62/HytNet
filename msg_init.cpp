
#include "messages.h"
#include "stdlib.h"
#include "string.h"
#include "enums.h"

#define htonl(A)    (((static_cast<unsigned long>(A) & 0xff000000) >> 24) | \
    ((static_cast<unsigned long>(A) & 0x00ff0000) >> 8)  | \
    ((static_cast<unsigned long>(A) & 0x0000ff00) << 8)  | \
    ((static_cast<unsigned long>(A) & 0x000000ff) << 24))

#define ntohl    htonl

hdpep_t 			 pep;
hdpep_no_pld_t pep_npld;
hrnp_t         restart;
hrnp_no_pld_t  ack_msg;
hrnp_no_pep_t  conn_req;
hrnp_no_pep_t  close_req;
hrnp_no_pld_t  pwr_up_chk_req;
hrnp_t               remote_monitor;
hrnp_t 				 uart_config_req;
hrnp_t 				 txt_notif_req;

hrnp_t               zone_ch;
hrnp_t               serialNum_req;
hrnp_t               chan_status;
hrnp_t               radio_id;
hrnp_t               call_req;
hrnp_t               ptt_press_req;
hrnp_t               ptt_release_req;
hrnp_t               br_cast_req;
hrnp_t               rtp_pld_req;
hrnp_t               dig_audio_rx_req;
hrnp_t               dig_audio_tx_req;
hrnp_t               key_notific_req;
hrnp_t               txt_notific_req;
hrnp_t               bc_txt_notific_req;
hrnp_t               br_cast_analog;

struct strMsgType_Ping  sMsgType_Ping;
struct strRMRpt         sRegMsgReport;
struct strMRRpt         sMRRpt;
struct strCRpt          sCallReport;
struct strCRp           sCallReply;
struct strCallControl   sCallControl;
extern quint32          Radio_Reg_State;
struct strCSRp          sCallStopReply;
struct strCCRpl         sChanChangeReply;
struct strCCRpt         sChanChangeReport;
struct strRldRpl        sReloadReplay;
struct strCtrlRpl       sCtrlReply;
struct strTxtMsg_SimpMsg sTxtMsg_SimpMsg;

strARSmsg        sARSmsg;

extern unsigned short checksum (unsigned char *ip);
extern void checksum_pep_no_pld(hrnp_no_pld_t *ip);
extern void checksum_pep(hrnp_t *ip);
extern quint8 tx[];
extern quint8 mu_law;
quint8 p[1] = {0};

static int cs;
quint32 ts_current;
quint32 period;
rtp_header rtp_hdr;

quint8 ars_answer[16];
quint8 ars_check[11 + 10];
quint8 txt_msg_tx[255];
void msg_init()
{
    sARSmsg.header.msgType = 0;
    sARSmsg.header.payloadLength[0] = 0;
    sARSmsg.header.payloadLength[1] = 10;
    sARSmsg.arsMsgType = RegMessage;// =0
    sARSmsg.csbk = 0;

    txt_msg_tx[0] = 0x09;

    // используем для подтверждений(len = 16) и запросов(len = 11)
    ars_answer[0] = 0x11; // rrs
    ars_answer[1] = 0x00; // opcode
    ars_answer[2] = 0x80;
    ars_answer[3] = 0x00; // size
    ars_answer[4] = 0x09;
    ars_answer[5] = 0x00; // ip
    ars_answer[6] = 0x00;
    ars_answer[7] = 0x00;
    ars_answer[8] = 0x00;
    ars_answer[9] = 0x00; // result
    ars_answer[10] = 0x00;
    ars_answer[11] = 0x00;
    ars_answer[12] = 0x00;
    ars_answer[13] = 0x01; // time
    ars_answer[12] = 0x00; // crc
    ars_answer[13] = 0x03;

    ars_check[0] = 0x11; // rrs
    ars_check[1] = 0x00; // opcode
    ars_check[2] = 0x02;
    ars_check[3] = 0x00; // size
    ars_check[4] = 0x04;
    ars_check[5] = 0x0A; // ip
    ars_check[6] = 0x00;
    ars_check[7] = 0x00;
    ars_check[8] = 0x00;
    ars_check[9] = 0x00; // crc
    ars_check[10] = 0x03;

//    ars_check[0] = 0x7E;
//    ars_check[1] = 0x04;
//    ars_check[2] = 0;
//    ars_check[3] = DATA;
//    ars_check[4] = 0x20;
//    ars_check[5] = 0x10;
//    ars_check[6] = 0;
//    ars_check[7] = 1;
//    ars_check[8] = 0;
//    ars_check[9] = 21;

//    ars_check[10] = 0x11; // rrs
//    ars_check[11] = 0x00; // opcode
//    ars_check[12] = 0x02;
//    ars_check[13] = 0x00; // size
//    ars_check[14] = 0x04;
//    ars_check[15] = 0x0A; // ip
//    ars_check[16] = 0x00;
//    ars_check[17] = 0x00;
//    ars_check[18] = 0x00;
//    ars_check[19] = 0x00; // crc
//    ars_check[20] = 0x03;


    pwr_up_chk_req.header = 0x7E;
    pwr_up_chk_req.version = 0x04;
    pwr_up_chk_req.block = 0;
    pwr_up_chk_req.opcode = DATA;
    pwr_up_chk_req.source = 0x20;
    pwr_up_chk_req.dest = 0x10;
    pwr_up_chk_req.packet_num[0] = 0;
    pwr_up_chk_req.packet_num[1] = 1;
    pwr_up_chk_req.length[0] = 0;
    pwr_up_chk_req.length[1] = 19;
    pwr_up_chk_req.pep_npld.MsgHdr = 0x02;
    pwr_up_chk_req.pep_npld.opcode[0] = 0xC6;
    pwr_up_chk_req.pep_npld.opcode[1] = 0x00;
    pwr_up_chk_req.pep_npld.num_of_bytes[0] = 0;
    pwr_up_chk_req.pep_npld.num_of_bytes[1] = 0;
    pwr_up_chk_req.pep_npld.Checksum = 0x00;
    pwr_up_chk_req.pep_npld.MsgEnd = 0x03;

    zone_ch.header = 0x7E;
    zone_ch.version = 0x04;
    zone_ch.block = 0;
    zone_ch.opcode = DATA;
    zone_ch.source = 0x20;
    zone_ch.dest = 0x10;
    zone_ch.packet_num[0] = 0;
    zone_ch.packet_num[1] = 3;
    zone_ch.length[0] = 0;
    zone_ch.length[1] = 12 + 12;
    zone_ch.pep.MsgHdr = 0x02;
    zone_ch.pep.opcode[0] = 0xC4;
    zone_ch.pep.opcode[1] = 0x00;
    zone_ch.pep.num_of_bytes[0] = 5;
    zone_ch.pep.num_of_bytes[1] = 0;
    zone_ch.pep.pld = (quint8*) malloc(zone_ch.pep.num_of_bytes[0]);
    zone_ch.pep.pld[0] = 0x01; //read message
    zone_ch.pep.pld[1] = 0x01;
    zone_ch.pep.pld[2] = 0x00;
    zone_ch.pep.pld[3] = 0x01;
    zone_ch.pep.pld[4] = 0x00;
    zone_ch.pep.Checksum = 0x00;
    zone_ch.pep.MsgEnd = 0x03;

    serialNum_req.header = 0x7E;
    serialNum_req.version = 0x04;
    serialNum_req.block = 0;
    serialNum_req.opcode = DATA;
    serialNum_req.source = 0x20;
    serialNum_req.dest = 0x10;
    serialNum_req.packet_num[0] = 0;
    serialNum_req.packet_num[1] = 3;
    serialNum_req.length[0] = 0;
    serialNum_req.length[1] = (12 + 5) + 1 + 2;
    serialNum_req.pep.MsgHdr = 0x02;
    serialNum_req.pep.opcode[0] = 0x01;
    serialNum_req.pep.opcode[1] = 0x02;
    serialNum_req.pep.num_of_bytes[0] = 1;
    serialNum_req.pep.num_of_bytes[1] = 0;
    serialNum_req.pep.pld = (quint8*) malloc(serialNum_req.pep.num_of_bytes[0]);
    serialNum_req.pep.pld[0] = 0x02; // s/n
    serialNum_req.pep.Checksum = 0x00;
    serialNum_req.pep.MsgEnd = 0x03;

    chan_status.header = 0x7E;
    chan_status.version = 0x04;
    chan_status.block = 0;
    chan_status.opcode = DATA;
    chan_status.source = 0x20;
    chan_status.dest = 0x10;
    chan_status.packet_num[0] = 0;
    chan_status.packet_num[1] = 3;
    chan_status.length[0] = 0;
    chan_status.length[1] = (12 + 5) + 2 + 2;
    chan_status.pep.MsgHdr = 0x02;
    chan_status.pep.opcode[0] = 0xE7;
    chan_status.pep.opcode[1] = 0x00;
    chan_status.pep.num_of_bytes[0] = 2;
    chan_status.pep.num_of_bytes[1] = 0;
    chan_status.pep.pld = (quint8*) malloc(chan_status.pep.num_of_bytes[0]);
    chan_status.pep.pld[0] = 0x07; // channel status
    chan_status.pep.pld[1] = 0x00;
    chan_status.pep.Checksum = 0x00;
    chan_status.pep.MsgEnd = 0x03;

    radio_id.header = 0x7E;
    radio_id.version = 0x04;
    radio_id.block = 0;
    radio_id.opcode = DATA;
    radio_id.source = 0x20;
    radio_id.dest = 0x10;
    radio_id.packet_num[0] = 0;
    radio_id.packet_num[1] = 3;
    radio_id.length[0] = 0;
    radio_id.length[1] = (12 + 5) + 1 + 2;
    radio_id.pep.MsgHdr = 0x02;
    radio_id.pep.opcode[0] = 0x52;
    radio_id.pep.opcode[1] = 0x04;
    radio_id.pep.num_of_bytes[0] = 1;
    radio_id.pep.num_of_bytes[1] = 0;
    radio_id.pep.pld = (quint8*) malloc(radio_id.pep.num_of_bytes[0]);
    radio_id.pep.pld[0] = 0x00; // radio id
    radio_id.pep.Checksum = 0x00;
    radio_id.pep.MsgEnd = 0x03;

    key_notific_req.header = 0x7E;
    key_notific_req.version = 0x04;
    key_notific_req.block = 0;
    key_notific_req.opcode = DATA;
    key_notific_req.source = 0x20;
    key_notific_req.dest = 0x10;
    key_notific_req.packet_num[0] = 0;
    key_notific_req.packet_num[1] = 3;
    key_notific_req.length[0] = 0;
    key_notific_req.length[1] = (12 + 5) + 7 + 2;;
    key_notific_req.pep.MsgHdr = 0x02;
    key_notific_req.pep.opcode[0] = 0xE4;
    key_notific_req.pep.opcode[1] = 0x10;
    key_notific_req.pep.num_of_bytes[0] = 7;
    key_notific_req.pep.num_of_bytes[1] = 0;
    key_notific_req.pep.pld = (quint8*) malloc(key_notific_req.pep.num_of_bytes[0]);
    key_notific_req.pep.pld[0] = 3;  // req id
    key_notific_req.pep.pld[1] = 3;
    key_notific_req.pep.pld[2] = 2;
    key_notific_req.pep.pld[3] = 0x18;
    key_notific_req.pep.pld[4] = 1;
    key_notific_req.pep.pld[5] = 0x19;
    key_notific_req.pep.pld[6] = 1;
    key_notific_req.pep.Checksum = 0x00;
    key_notific_req.pep.MsgEnd = 0x03;

    bc_txt_notific_req.header = 0x7E;//broadcast message configuration
    bc_txt_notific_req.version = 0x04;
    bc_txt_notific_req.block = 0;
    bc_txt_notific_req.opcode = DATA;
    bc_txt_notific_req.source = 0x20;
    bc_txt_notific_req.dest = 0x10;
    bc_txt_notific_req.packet_num[0] = 0;
    bc_txt_notific_req.packet_num[1] = 3;
    bc_txt_notific_req.length[0] = 0;
    bc_txt_notific_req.length[1] = (12 + 5) + 8 + 2;;
    bc_txt_notific_req.pep.MsgHdr = 0x02;
    bc_txt_notific_req.pep.opcode[0] = 0x47;
    bc_txt_notific_req.pep.opcode[1] = 0x18;
    bc_txt_notific_req.pep.num_of_bytes[0] = 8;
    bc_txt_notific_req.pep.num_of_bytes[1] = 0;
    bc_txt_notific_req.pep.pld = (quint8*) malloc(bc_txt_notific_req.pep.num_of_bytes[0]);
    bc_txt_notific_req.pep.pld[0] = 0;// 0-normal txt   1-short txt
    bc_txt_notific_req.pep.Checksum = 0x00;
    bc_txt_notific_req.pep.MsgEnd = 0x03;

    txt_notific_req.header = 0x7E;
    txt_notific_req.version = 0x04;
    txt_notific_req.block = 0;
    txt_notific_req.opcode = DATA;
    txt_notific_req.source = 0x20;
    txt_notific_req.dest = 0x10;
    txt_notific_req.packet_num[0] = 0;
    txt_notific_req.packet_num[1] = 3;
    txt_notific_req.length[0] = 0;
    txt_notific_req.length[1] = (12 + 5) + 1 + 2;;
    txt_notific_req.pep.MsgHdr = 0x02;
    txt_notific_req.pep.opcode[0] = 0xD0;
    txt_notific_req.pep.opcode[1] = 0x10;
    txt_notific_req.pep.num_of_bytes[0] = 1;
    txt_notific_req.pep.num_of_bytes[1] = 0;
    txt_notific_req.pep.pld = (quint8*) malloc(txt_notific_req.pep.num_of_bytes[0]);
    txt_notific_req.pep.pld[0] = 1;
    txt_notific_req.pep.Checksum = 0x00;
    txt_notific_req.pep.MsgEnd = 0x03;

    rtp_pld_req.header = 0x7E;
    rtp_pld_req.version = 0x04;
    rtp_pld_req.block = 0;
    rtp_pld_req.opcode = DATA;
    rtp_pld_req.source = 0x20;
    rtp_pld_req.dest = 0x10;
    rtp_pld_req.packet_num[0] = 0;
    rtp_pld_req.packet_num[1] = 3;
    rtp_pld_req.length[0] = 0;
    rtp_pld_req.length[1] = (12 + 5) + 5 + 2;
    rtp_pld_req.pep.MsgHdr = 0x02;
    rtp_pld_req.pep.opcode[0] = 0x19;
    rtp_pld_req.pep.opcode[1] = 0x04;
    rtp_pld_req.pep.num_of_bytes[0] = 5;
    rtp_pld_req.pep.num_of_bytes[1] = 0;
    rtp_pld_req.pep.pld = (quint8*) malloc(rtp_pld_req.pep.num_of_bytes[0]);
    rtp_pld_req.pep.pld[0] = 0x08; // uLaw
    rtp_pld_req.pep.Checksum = 0x00;
    rtp_pld_req.pep.MsgEnd = 0x03;


    dig_audio_rx_req.header = 0x7E;
    dig_audio_rx_req.version = 0x04;
    dig_audio_rx_req.block = 0;
    dig_audio_rx_req.opcode = DATA;
    dig_audio_rx_req.source = 0x20;
    dig_audio_rx_req.dest = 0x10;
    dig_audio_rx_req.packet_num[0] = 0;
    dig_audio_rx_req.packet_num[1] = 3;
    dig_audio_rx_req.length[0] = 0;
    dig_audio_rx_req.length[1] = (12 + 5) + 2 + 2;
    dig_audio_rx_req.pep.MsgHdr = 0x02;
    dig_audio_rx_req.pep.opcode[0] = 0xDF;
    dig_audio_rx_req.pep.opcode[1] = 0x00;
    dig_audio_rx_req.pep.num_of_bytes[0] = 2;
    dig_audio_rx_req.pep.num_of_bytes[1] = 0;
    dig_audio_rx_req.pep.pld = (quint8*) malloc(dig_audio_rx_req.pep.num_of_bytes[0]);
    dig_audio_rx_req.pep.pld[0] = 0x01; //  rx
    dig_audio_rx_req.pep.pld[1] = 0x01; //  from air
    dig_audio_rx_req.pep.Checksum = 0x00;
    dig_audio_rx_req.pep.MsgEnd = 0x03;

    dig_audio_tx_req.header = 0x7E;
    dig_audio_tx_req.version = 0x04;
    dig_audio_tx_req.block = 0;
    dig_audio_tx_req.opcode = DATA;
    dig_audio_tx_req.source = 0x20;
    dig_audio_tx_req.dest = 0x10;
    dig_audio_tx_req.packet_num[0] = 0;
    dig_audio_tx_req.packet_num[1] = 3;
    dig_audio_tx_req.length[0] = 0;
    dig_audio_tx_req.length[1] = (12 + 5) + 2 + 2;
    dig_audio_tx_req.pep.MsgHdr = 0x02;
    dig_audio_tx_req.pep.opcode[0] = 0xDF;
    dig_audio_tx_req.pep.opcode[1] = 0x00;
    dig_audio_tx_req.pep.num_of_bytes[0] = 2;
    dig_audio_tx_req.pep.num_of_bytes[1] = 0;
    dig_audio_tx_req.pep.pld = (quint8*) malloc(dig_audio_tx_req.pep.num_of_bytes[0]);
    dig_audio_tx_req.pep.pld[0] = 0x00; //  tx
    dig_audio_tx_req.pep.pld[1] = 0x01; //  from air
    dig_audio_tx_req.pep.Checksum = 0x00;
    dig_audio_tx_req.pep.MsgEnd = 0x03;

    br_cast_req.header = 0x7E;
    br_cast_req.version = 0x04;
    br_cast_req.block = 0;
    br_cast_req.opcode = DATA;
    br_cast_req.source = 0x20;
    br_cast_req.dest = 0x10;
    br_cast_req.packet_num[0] = 0;
    br_cast_req.packet_num[1] = 3;
    br_cast_req.length[0] = 0;
    br_cast_req.length[1] = (12 + 5) + 5 + 2;

    br_cast_req.pep.MsgHdr = 0x02;
    br_cast_req.pep.opcode[0] = 0xC9;
    br_cast_req.pep.opcode[1] = 0x10;
    br_cast_req.pep.num_of_bytes[0] = 5;
    br_cast_req.pep.num_of_bytes[1] = 0;
    br_cast_req.pep.pld = (quint8*) malloc(br_cast_req.pep.num_of_bytes[0]);
    br_cast_req.pep.pld[0] = 0x02; // size
    br_cast_req.pep.pld[1] = 0x00; // rx
    br_cast_req.pep.pld[2] = 0x01; // notify
    br_cast_req.pep.pld[3] = 0x01; // tx
    br_cast_req.pep.pld[4] = 0x01; // notify
    br_cast_req.pep.Checksum = 0x00;
    br_cast_req.pep.MsgEnd = 0x03;
}
