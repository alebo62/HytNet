#ifndef TCP_H
#define TCP_H

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QDebug>
#include <QHostAddress>
#include <QTimer>
#include <QDateTime>

#define DBG
#define DBG_BA

const quint16  RRS = 3002;
const quint16  GPS = 3003;
const quint16  TMS = 3004;
const quint16  RCP = 3005;
const quint16  TLM = 3006;
const quint16  DTR = 3007;
const quint16  SDM = 3009;



class TCP : public QObject
{
    Q_OBJECT
public:
    explicit TCP(QObject *parent = nullptr);
    QTcpSocket tcp_srv;
    QUdpSocket udp_srv;
    QUdpSocket udpRRS_3002;
    QUdpSocket udpGPS_3003;
    QUdpSocket udpTMS_3004;
    QUdpSocket udpRCP_3005;
    QUdpSocket udpTLM_3006;
    QUdpSocket udpDTR_3007;
    QUdpSocket udpSDM_3009;

    QTimer rad_conn_tim;
    QTimer tcp_conn_tim;
    QTimer ping_tim;
    QTimer reload_tim;
    QTimer radio_check_tim;
    QTimer monitor_tim;
    QTimer rx_tim;
    QTimer tx_tim;

    void prepare_udp();
    void receive_sound();
    void rcv_tcpRRS();
    void rcv_tcpTMS();
    void rcv_tcpGPS();
    void rcv_tcpRCP();
    double get_latitude(quint32);
    double get_longitude(quint32);
    float get_speed(quint32);
    quint32 get_direction(quint32);
    quint8 pi_num;

signals:

public slots:
    void rcv_udpRRS();
    void rcv_udpGPS();
    void rcv_udpTMS();
    void rcv_udpRCP();
    void rcv_udpTLM();
    void rcv_udpDTR();
    void rcv_udpSDM();
    void rad_conn_tim_slot();

    void tcp_conn();
    void tcp_disconn();
    void tcp_receive();
    void udp_srv_slot();

    void tcp_conn_tim_slot();
    void ping_tim_slot();
    void reload_tim_slot();
    void radio_check_tim_slot();
    void monitor_tim_slot();
    void rx_tim_slot();
    void tx_tim_slot();


};

#endif // TCP_H
