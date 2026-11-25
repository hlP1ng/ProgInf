#ifndef MAVLINKMESSAGEPARSER_H
#define MAVLINKMESSAGEPARSER_H

#include <QtGlobal>
#include <QByteArray>
#include <QString>

enum MavCmd : quint16 {
    MAV_CMD_NAV_LAND          = 21,
    MAV_CMD_NAV_TAKEOFF       = 22,
    MAV_CMD_DO_PAUSE_CONTINUE = 193
};

enum MavResult : quint8 {
    MAV_RESULT_ACCEPTED             = 0,
    MAV_RESULT_TEMPORARILY_REJECTED = 1,
    MAV_RESULT_DENIED               = 2,
    MAV_RESULT_UNSUPPORTED          = 3,
    MAV_RESULT_FAILED               = 4,
    MAV_RESULT_IN_PROGRESS          = 5
};

enum MavMsgId : quint32 {
    MAVLINK_MSG_ID_HEARTBEAT           = 0,
    MAVLINK_MSG_ID_GLOBAL_POSITION_INT = 33,
    MAVLINK_MSG_ID_COMMAND_LONG        = 76,
    MAVLINK_MSG_ID_COMMAND_ACK         = 77
};

#pragma pack(push, 1)
struct SimpleTelemetryPayload
{
    quint32 timestampMs;
    qint32 latitudeE7;
    qint32 longitudeE7;
    qint32 altitudeMm;
    quint16 flightMode;
};

struct HeartbeatPayload
{
    quint16 flightMode;
    quint8 systemStatus;
};

struct CommandLongPayload
{
    float param1;
    float param2;
    float param3;
    float param4;
    float param5;
    float param6;
    float param7;
    quint16 command;
    quint8 targetSystem;
    quint8 targetComponent;
    quint8 confirmation;
};

struct CommandAckPayload
{
    quint16 command;
    quint8 result;
};
#pragma pack(pop)

struct ParsedMavlinkMessage
{
    quint8 magic = 0;
    quint8 payloadLength = 0;
    quint8 sequence = 0;
    quint8 systemId = 0;
    quint8 componentId = 0;
    quint32 messageId = 0;
    QByteArray payload;
};

class MavlinkMessageParser
{
public:
    static bool isValid(const QByteArray &data, QString *errorReason = nullptr);
    static bool parse(const QByteArray &data, ParsedMavlinkMessage &outMsg, QString *errorReason = nullptr);

    static QByteArray createHeartbeatPacket(quint8 seq, quint8 sysId, quint8 compId, quint16 flightMode);
    static QByteArray createTelemetryPacket(quint8 seq, quint8 sysId, quint8 compId, const SimpleTelemetryPayload &telemetry);
    static QByteArray createCommandLongPacket(quint8 seq, quint8 sysId, quint8 compId,
                                              quint8 targetSysId, quint8 targetCompId,
                                              quint16 command, float param7 = 0.0f);
    static QByteArray createCommandAckPacket(quint8 seq, quint8 sysId, quint8 compId,
                                             quint16 command, quint8 result);

    static QString flightModeToString(quint16 mode);
    static quint16 calculateCrc16(const QByteArray &data, int startOffset, int length);

private:
    static QByteArray buildPacket(quint8 seq, quint8 sysId, quint8 compId, quint32 msgId, const QByteArray &payload);
};

#endif // MAVLINKMESSAGEPARSER_H