#include "tcp.h"
#include "enums.h"

extern QByteArray ba;
/*
 *Отправляем на станцию --->
 *
    MsgHdr(0x08)
    Opcode
        0xA0 Standard Location Immediate Report
            0x01 (Request)          RequestID[4]+RadioIP[4]  --->
        0xB0 --- Emergency Location Reporting Service
            0x01 (ReportStopReq)    RequestID[4]+RadioIP[4]  --->
        0xC0 --- Triggered Location Reporting Service
            0x01 (ReportReq)        RequestID[4]+RadioIP[4] + StartTime[14] + StopTime[14] + Interval[8] --->
            0x04 (ReportStopReq)    RequestID[4]+RadioIP[4]   --->
        0xD0 --- Condition Triggered Reporting Service
            0x01 (ReportReq)        RequestID[4]+RadioIP[4] + TriggerType[1]+Distance[4]+StartTime[14] + StopTime[14] + Interval[8]+MaxInterval[8]
            0x11 (QuickGPSReq)      RequestID[4]+RadioIP[4]+CmdType[1]+Field[N]
        0xE0 --- RSSI Report Configuring Service
            0x01(RSSIReportConfReq) RequestID[4]+RadioIP[4]+ReportType[2]
    #of bytes
            0xA0 - Standard Location Immediate Service  8 - 50 - 52 - 49(31-bluetooth)
            0xB0 - Emergency Location Reporting Service 8 - 10 - 45 - 46(28-bluetooth)
            0xC0 - Triggered Location Reporting Service 44 - 10 - 56 - 8 - 10
            0xD0 - Condition Triggered Reporting Service  49 - 9+???
            0xE0 - RSSI Report Configuring Service   10
    Payload
        ...
    Checksum
    MsgEnd （0x03）


    crc(uchar* p, len)
    len = p[4] + 7  !!!!!!!!!!!!!!!!

*/


QByteArray ba_3003;
void TCP::rcv_udpGPS()
{
    ba_3003.resize(static_cast<int>(udpGPS_3003.pendingDatagramSize()));
    udpGPS_3003.readDatagram(ba_3003.data(), ba_3003.size());

    switch (ba_3003.at(1)) {
    case StdLocImmReport:
        switch (ba_3003.at(1)) {
        case 0x02:  // RequestID[4]+RadioIP[4] + Result[2]+ GPSData[40]

            break;
        case 0x03: // RequestID[4]+RadioIP[4] + Result[2]+GPSData[40]+RSSIValue[2]

            break;
        case 0x04:// RadioIP[4]+EmegencyType[1]+DataType[1]+Data[n]

            break;
        }

        break;
    case EmgLocReportSrv:
        switch (ba_3003.at(1)) {
        case 0x02: //  RequestID[4]+RadioIP[4] + Result[2]

            break;
        case 0x03: // RadioIP[4] + EmegencyType[1]+GPSData[40]

            break;
        case 0x04: // RadioIP[4]+EmegencyType[1]+DataType[1]+Data[n]

            break;
        default:

            break;
        }
        break;
    case TrigLocReportSrv:
        switch (ba_3003.at(1)) {
        case 0x02: // RequestID[4]+RadioIP[4] + Result[2]

            break;
        case 0x03:  // RequestID[4]+RadioIP[4] + TimeRemaining[8] + GPSData[40]

            break;
        case 0x05://  RequestID[4]+RadioIP[4] + Result[2]

            break;
        default:

            break;
        }
        break;
    case CondTrigReportSrv:
        switch (ba_3003.at(1)) {
        case 0x02: //RequestID[4]+RadioIP[4] + TriggerType[1]+Result[2]

            break;
        case 0x03: // RequestID[4]+RadioIP[4] + GPSData[40]

            break;
        case 0x04:// RequestID[4]+RadioIP[4] + GPSData[40]+RSSIValue[2]

            break;
        case 0x05: // RequestID[4]+RadioIP[4]+DataType[1]+Data[n]

            break;
        case 0x12: // RequestID[4]+RadioIP[4]+ CmdType[1]+Result[2]

            break;
        default:

            break;
        }
        break;
    case RssiReportConfigSrv:
        switch (ba_3003.at(1)) {
        case 0x02: // RequestID[4]+RadioIP[4]+ReportType[2]+Result[2]

            break;
        default:

            break;
        }
        break;
    default:
        break;
    }
}

/*
 Это для передачи на сервер --->

    IronAgentMsgType  1 байт (0x02)
    PayloadLength     2 байта
    Reserved          1 byte

    LocationMsgType 1 байт(4)
        ImmediateLocationReport     (0x02)  --->
        TriggeredLocationAnswer     (0x04)  --->
        TriggeredLocationReport     (0x05)  --->
        TriggeredLocationStopAnswer (0x07)  --->

    RadioId 4 байта	  (5..8)
    RequestId 4 байта (9..12)
    Payload  (13..)
        int Result – результат, + LocationData – структура, описанная ниже (44)
        int Result – результат установки триггера (4)
        int Result – результат, + LocationData – структура, описанная ниже (44)
        int Result – результат установки триггера (4)


*/

void TCP::rcv_tcpGPS()
{
//    IronAgentMsgType  1 байт (0x02)
//    PayloadLength     2 байта
//    Reserved          1 byte

    switch (ba.at(4)) {   // RadioId 4 байта(5..8) +  RequestId 4 байта (9..12)
    case ImmedLocRequest: // bool UseCsbk – использовать CSBK при передаче

        break;
    case TrigLocaRequest: // u16 interval – инт между отчетами + bool UseCsbk – исп. CSBK при передаче

        break;
    case TrigLocStopRequest: // Отсутствует

        break;
    default:
        break;
    }
    //    addr = "12." + QString::number(static_cast<quint8>(ba1.at(6))) + "." + QString::number(static_cast<quint8>(ba1.at(7))) + "." + QString::number(static_cast<quint8>(ba1.at(8))) ;

    //    if(ba1.at(4) == ImmedLocRequest )
    //    {
    //        memcpy(imm_loc_req + 5, ba1.data() + 10, 3);// copy request id
    //        udp1_4001.writeDatagram(reinterpret_cast<char*>(imm_loc_req), 11, QHostAddress(addr), 4001);
    //#ifdef DBG
    //        qDebug() << QDateTime::currentDateTime().toString(" yyyy-MM-dd hh:mm:ss ") << "ImmedLocRequest_1 for " << addr;
    //#endif
    //    }
    //    else if(ba1.at(4) == TrigLocaRequest)
    //    {
    //        memcpy(trig_loc_req + 5, ba1.data() + 10, 3);// copy request id
    //        if(ba1.at(13) == 0)
    //        {
    //            if(static_cast<quint8>(ba1.at(14)) > 127)
    //            {
    //                trig_loc_req[1] = 13;
    //                trig_loc_req[13] = static_cast<quint8>(ba1.at(14));
    //                trig_loc_req[14] = 0x01;// copy interval
    //                udp1_4001.writeDatagram(reinterpret_cast<char*>(trig_loc_req), 15, QHostAddress(addr), 4001);
    //            }
    //            else
    //            {
    //                trig_loc_req[1] = 12;
    //                trig_loc_req[13] = static_cast<quint8>(ba1.at(14));
    //                udp1_4001.writeDatagram(reinterpret_cast<char*>(trig_loc_req), 14, QHostAddress(addr), 4001);
    //            }
    //        }
    //        else
    //        {
    //            trig_loc_req[1] = 13;
    //            trig_loc_req[13] = static_cast<quint8>(ba1.at(14));// interval  8min
    //            trig_loc_req[14] = 0x03;// copy interval
    //            udp1_4001.writeDatagram(reinterpret_cast<char*>(trig_loc_req), 15, QHostAddress(addr), 4001);
    //        }
    //#ifdef DBG
    //        qDebug() << QDateTime::currentDateTime().toString(" yyyy-MM-dd hh:mm:ss ") << "TrigLocRequest_1 for " << addr;
    //#endif
    //    }
    //    else if(ba1.at(4) == TrigLocStopRequest)
    //    {
    //        memcpy(trig_loc_stop_req + 5, ba1.data() + 10, 3);// copy requset id
    //        udp1_4001.writeDatagram(reinterpret_cast<char*>(trig_loc_stop_req), 8, QHostAddress(addr), 4001);
    //#ifdef DBG
    //        qDebug() << QDateTime::currentDateTime().toString(" yyyy-MM-dd hh:mm:ss ") << "TrigLocStopRequest_1 for " << addr;
    //#endif
    //    }

}
