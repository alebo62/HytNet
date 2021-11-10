#include "tcp.h"
#include "variables.h"

extern void route_rpi();
extern void msg_init();
extern unsigned char conn[];
extern QString host;
extern void crc(unsigned char*, int);


TCP::TCP(QObject *parent) : QObject(parent)
{
    route_rpi();
    prepare_udp();
    msg_init();
    connect(&rad_conn_tim, SIGNAL(timeout()), this, SLOT(rad_conn_tim_slot()));
    connect(&tcp_conn_tim, SIGNAL(timeout()), this, SLOT(tcp_conn_tim_slot()));
    connect(&ping_tim,     SIGNAL(timeout()), this, SLOT(ping_tim_slot()));
    connect(&reload_tim,   SIGNAL(timeout()), this, SLOT(reload_tim_slot()));
    connect(&radio_check_tim,   SIGNAL(timeout()), this, SLOT(radio_check_tim_slot()));
    connect(&monitor_tim,   SIGNAL(timeout()), this, SLOT(monitor_tim_slot()));
    connect(&rx_tim,   SIGNAL(timeout()), this, SLOT(rx_tim_slot()));
    connect(&tx_tim,   SIGNAL(timeout()), this, SLOT(tx_tim_slot()));
    connect(&sound_tim,   SIGNAL(timeout()), this, SLOT(sound_tim_slot()));

    connect(&tcp_srv,SIGNAL(readyRead()),this,SLOT(tcp_receive()));
    connect(&tcp_srv,SIGNAL(disconnected()),this,SLOT(tcp_disconn()));
    connect(&tcp_srv,SIGNAL(connected()),this,SLOT(tcp_conn()));

    memcpy(aud_tx ,  sound_msg, 52);
    rad_conn_tim.setInterval(3000);
    monitor_tim.setInterval(18000);
    rad_conn_tim.start();
    udpRCP_3005.writeDatagram((char*)conn, 12, QHostAddress(host), RCP);
    tcp_conn_tim.setInterval(3000);
    ping_tim.setInterval(11000);
    reload_tim.setInterval(1000);
    radio_check_tim.setInterval(2000);
    rx_tim.setInterval(5000);
    tx_tim.setInterval(5000);
    sound_tim.setInterval(15);

    //qDebug()<< dest_rrs;
}




void TCP::prepare_udp()
{
    connect(&udpRRS_3002, SIGNAL(readyRead()), this, SLOT(rcv_udpRRS()));
    udpRRS_3002.bind(QHostAddress(MY_RI_ADDR), RRS);

    connect(&udpGPS_3003, SIGNAL(readyRead()), this, SLOT(rcv_udpGPS()));
    udpGPS_3003.bind(QHostAddress(MY_RI_ADDR), GPS);

    connect(&udpTMS_3004, SIGNAL(readyRead()), this, SLOT(rcv_udpTMS()));
    udpTMS_3004.bind(QHostAddress(MY_RI_ADDR), TMS);

    connect(&udpRCP_3005, SIGNAL(readyRead()), this, SLOT(rcv_udpRCP()));
    udpRCP_3005.bind(QHostAddress(MY_RI_ADDR), RCP);

    connect(&udpTLM_3006, SIGNAL(readyRead()), this, SLOT(rcv_udpTLM()));
    udpTLM_3006.bind(QHostAddress(MY_RI_ADDR), TLM);

    connect(&udpDTR_3007, SIGNAL(readyRead()), this, SLOT(rcv_udpDTR()));
    udpDTR_3007.bind(QHostAddress(MY_RI_ADDR), DTR);

    connect(&udpSDM_3009, SIGNAL(readyRead()), this, SLOT(rcv_udpSDM()));
    udpSDM_3009.bind(QHostAddress(MY_RI_ADDR), SDM);
}

