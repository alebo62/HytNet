#include "cstring"
#include "tcp.h"
#include "messages.h"


using namespace std;

extern QByteArray ba;
extern QString hadrr, host;
extern quint16 hyterus_port;

#define TS_DELTA 160
#define RTP_HDR_LEN 12
#define htonl(A)    (((static_cast<unsigned long>(A) & 0xff000000) >> 24) | \
    ((static_cast<unsigned long>(A) & 0x00ff0000) >> 8)  | \
    ((static_cast<unsigned long>(A) & 0x0000ff00) << 8)  | \
    ((static_cast<unsigned long>(A) & 0x000000ff) << 24))

unsigned char temp_8[532];
quint16 seq_num;
extern quint32 ts_current;
extern rtp_header rtp_hdr;
unsigned char aud_tx[532];
QByteArray baUdpHyt;
quint16 len;
quint32 timestamp;

void TCP::udp_srv_slot()
{
    baUdpHyt.resize(static_cast<int>(udp_srv.pendingDatagramSize()));
    udp_srv.readDatagram(baUdpHyt.data(), baUdpHyt.size());

    memmove(aud_tx + 52 , baUdpHyt.data() + 28  , 480);
    seq_num++;
    aud_tx[14] = seq_num >> 8;
    aud_tx[15] = seq_num & 0xFF;
    timestamp ++;
    aud_tx[16] = timestamp >> 24;
    aud_tx[17] = timestamp >> 16 & 0xFF;
    aud_tx[18] = (timestamp >> 8) & 0xFF;
    aud_tx[19] = timestamp & 0xFF;

    udpRCP_3005.writeDatagram((char*)aud_tx, 532, QHostAddress(host), RCP);
}

void TCP::receive_sound()
{
    ts_current += TS_DELTA;
    rtp_hdr.timestamp = htonl(ts_current);
    seq_num++;
    rtp_hdr.seq_no[0] = seq_num >> 8;
    rtp_hdr.seq_no[1] = seq_num & 0xff;

    memcpy(temp_8 ,reinterpret_cast<quint8*>(&rtp_hdr), RTP_HDR_LEN );
    memcpy(temp_8 + RTP_HDR_LEN , ba.data() + 52, 480 );

    udp_srv.writeDatagram( reinterpret_cast<char*>(temp_8), 480 + RTP_HDR_LEN, QHostAddress(hadrr), hyterus_port);
}

