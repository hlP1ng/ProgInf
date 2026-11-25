#include "mavlinkmessageparser.h"
#include <cmath>
#include <cstring>

quint16 MavlinkMessageParser::calculateCrc16(const QByteArray &data, int startOffset, int length)
{
    quint16 crc = 0xFFFF;
    for (int i = startOffset; i < startOffset + length && i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data.at(i));
        quint8 tmp = byte ^ static_cast<quint8>(crc & 0xFF);
        tmp ^= (tmp << 4);
        crc = (crc >> 8) ^ (static_cast<quint16>(tmp) << 8)
              ^ (static_cast<quint16>(tmp) << 3)
              ^ (static_cast<quint16>(tmp) >> 4);
    }
    return crc;
}

bool MavlinkMessageParser::isValid(const QByteArray &data, QString *errorReason)
{
    if (data.size() < 12) {
        if (errorReason) *errorReason = "Слишком короткий MAVLink-пакет (< 12 байт)";
        return false;
    }

    const quint8 magic = static_cast<quint8>(data.at(0));
    if (magic != 0xFD && magic != 0xFE) {
        if (errorReason) *errorReason = "Неверный стартовый байт (ожидался 0xFD)";
        return false;
    }

    const quint8 payloadLen = static_cast<quint8>(data.at(1));
    const int expectedTotalSize = 10 + payloadLen + 2;
    if (data.size() != expectedTotalSize) {
        if (errorReason) *errorReason = "Неверный размер пакета (не совпадает с полем длины)";
        return false;
    }

    const quint16 expectedCrc = calculateCrc16(data, 1, 9 + payloadLen);
    const quint8 crcLow = static_cast<quint8>(data.at(10 + payloadLen));
    const quint8 crcHigh = static_cast<quint8>(data.at(11 + payloadLen));
    const quint16 actualCrc = static_cast<quint16>(crcLow) | (static_cast<quint16>(crcHigh) << 8);

    if (expectedCrc != actualCrc) {
        if (errorReason) *errorReason = "Поврежденный пакет: неверная контрольная сумма CRC";
        return false;
    }

    return true;
}

bool MavlinkMessageParser::parse(const QByteArray &data, ParsedMavlinkMessage &outMsg, QString *errorReason)
{
    if (!isValid(data, errorReason)) {
        return false;
    }

    outMsg.magic         = static_cast<quint8>(data.at(0));
    outMsg.payloadLength = static_cast<quint8>(data.at(1));
    outMsg.sequence      = static_cast<quint8>(data.at(4));
    outMsg.systemId      = static_cast<quint8>(data.at(5));
    outMsg.componentId   = static_cast<quint8>(data.at(6));

    const quint32 msgId0 = static_cast<quint8>(data.at(7));
    const quint32 msgId1 = static_cast<quint8>(data.at(8));
    const quint32 msgId2 = static_cast<quint8>(data.at(9));
    outMsg.messageId     = msgId0 | (msgId1 << 8) | (msgId2 << 16);

    outMsg.payload = data.mid(10, outMsg.payloadLength);
    return true;
}

QByteArray MavlinkMessageParser::buildPacket(quint8 seq, quint8 sysId, quint8 compId, quint32 msgId, const QByteArray &payload)
{
    QByteArray packet;
    packet.reserve(10 + payload.size() + 2);

    packet.append(static_cast<char>(0xFD));
    packet.append(static_cast<char>(payload.size()));
    packet.append(static_cast<char>(0x00));
    packet.append(static_cast<char>(0x00));
    packet.append(static_cast<char>(seq));
    packet.append(static_cast<char>(sysId));
    packet.append(static_cast<char>(compId));
    packet.append(static_cast<char>(msgId & 0xFF));
    packet.append(static_cast<char>((msgId >> 8) & 0xFF));
    packet.append(static_cast<char>((msgId >> 16) & 0xFF));

    packet.append(payload);

    const quint16 crc = calculateCrc16(packet, 1, 9 + payload.size());
    packet.append(static_cast<char>(crc & 0xFF));
    packet.append(static_cast<char>((crc >> 8) & 0xFF));

    return packet;
}

QByteArray MavlinkMessageParser::createHeartbeatPacket(quint8 seq, quint8 sysId, quint8 compId, quint16 flightMode)
{
    HeartbeatPayload hb{};
    hb.flightMode = flightMode;
    hb.systemStatus = 4;

    QByteArray payload(reinterpret_cast<const char*>(&hb), sizeof(hb));
    return buildPacket(seq, sysId, compId, MAVLINK_MSG_ID_HEARTBEAT, payload);
}

QByteArray MavlinkMessageParser::createTelemetryPacket(quint8 seq, quint8 sysId, quint8 compId, const SimpleTelemetryPayload &telemetry)
{
    QByteArray payload(reinterpret_cast<const char*>(&telemetry), sizeof(telemetry));
    return buildPacket(seq, sysId, compId, MAVLINK_MSG_ID_GLOBAL_POSITION_INT, payload);
}

QByteArray MavlinkMessageParser::createCommandLongPacket(quint8 seq, quint8 sysId, quint8 compId,
                                                         quint8 targetSysId, quint8 targetCompId,
                                                         quint16 command, float param7)
{
    CommandLongPayload cmd{};
    cmd.param1 = 0.0f;
    cmd.param2 = 0.0f;
    cmd.param3 = 0.0f;
    cmd.param4 = NAN;
    cmd.param5 = NAN;
    cmd.param6 = NAN;
    cmd.param7 = param7;
    cmd.command = command;
    cmd.targetSystem = targetSysId;
    cmd.targetComponent = targetCompId;
    cmd.confirmation = 0;

    QByteArray payload(reinterpret_cast<const char*>(&cmd), sizeof(cmd));
    return buildPacket(seq, sysId, compId, MAVLINK_MSG_ID_COMMAND_LONG, payload);
}

QByteArray MavlinkMessageParser::createCommandAckPacket(quint8 seq, quint8 sysId, quint8 compId,
                                                        quint16 command, quint8 result)
{
    CommandAckPayload ack{};
    ack.command = command;
    ack.result = result;

    QByteArray payload(reinterpret_cast<const char*>(&ack), sizeof(ack));
    return buildPacket(seq, sysId, compId, MAVLINK_MSG_ID_COMMAND_ACK, payload);
}

QString MavlinkMessageParser::flightModeToString(quint16 mode)
{
    switch (mode) {
    case 0: return "MANUAL";
    case 1: return "STABILIZED";
    case 2: return "ALT_HOLD";
    case 3: return "POS_HOLD";
    case 4: return "GUIDED";
    case 5: return "AUTO";
    case 6: return "LAND";
    default: return "UNKNOWN";
    }
}