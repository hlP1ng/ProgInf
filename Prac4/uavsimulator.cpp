#include "uavsimulator.h"
#include "mavlinkmessageparser.h"
#include <QDateTime>
#include <QNetworkDatagram>
#include <QHostAddress>
#include <cstring>

UavSimulator::UavSimulator(QObject *parent)
    : QObject(parent)
{
    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(QHostAddress::LocalHost, 0);
    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &UavSimulator::onReadyRead);

    m_telemetryTimer = new QTimer(this);
    m_telemetryTimer->setInterval(100);
    connect(m_telemetryTimer, &QTimer::timeout,
            this, &UavSimulator::sendTelemetry);
}

void UavSimulator::start(const QString &targetAddress, quint16 targetPort)
{
    m_targetAddress = targetAddress;
    m_targetPort = targetPort;
    m_tickCounter = 0;
    m_telemetryTimer->start();
}

void UavSimulator::stop()
{
    m_telemetryTimer->stop();
}

bool UavSimulator::isRunning() const
{
    return m_telemetryTimer->isActive();
}

void UavSimulator::corruptNextPacket()
{
    m_corruptNextPacket = true;
}

void UavSimulator::skipTwoPackets()
{
    m_sequence = static_cast<quint8>((m_sequence + 2) % 256);
}

void UavSimulator::setSuppressAck(bool suppress)
{
    m_suppressAck = suppress;
}

void UavSimulator::sendTelemetry()
{
    if (m_tickCounter % 30 == 0) {
        QByteArray hbDatagram = MavlinkMessageParser::createHeartbeatPacket(
            m_sequence++, m_systemId, m_componentId, m_flightMode);
        m_udpSocket->writeDatagram(hbDatagram, QHostAddress(m_targetAddress), m_targetPort);
    }
    m_tickCounter++;

    if (m_flightMode == 4) {
        if (m_altitude < m_targetAltitude) {
            m_altitude += 0.5;
            if (m_altitude > m_targetAltitude) m_altitude = m_targetAltitude;
        } else if (m_altitude > m_targetAltitude) {
            m_altitude -= 0.5;
            if (m_altitude < m_targetAltitude) m_altitude = m_targetAltitude;
        }
        m_latitude += 0.000005;
        m_longitude += 0.000005;
    } else if (m_flightMode == 6) {
        if (m_altitude > 0.0) {
            m_altitude -= 0.8;
            if (m_altitude <= 0.0) {
                m_altitude = 0.0;
                m_flightMode = 1;
            }
        }
    }

    SimpleTelemetryPayload telemetry{};
    telemetry.timestampMs = static_cast<quint32>(
        QDateTime::currentMSecsSinceEpoch() % 4294967295ULL);
    telemetry.latitudeE7  = static_cast<qint32>(m_latitude * 1e7);
    telemetry.longitudeE7 = static_cast<qint32>(m_longitude * 1e7);
    telemetry.altitudeMm  = static_cast<qint32>(m_altitude * 1000.0);
    telemetry.flightMode  = m_flightMode;

    QByteArray datagram = MavlinkMessageParser::createTelemetryPacket(
        m_sequence++, m_systemId, m_componentId, telemetry);

    if (m_corruptNextPacket) {
        datagram[3] = static_cast<char>(static_cast<quint8>(datagram[3]) ^ 0xFF);
        m_corruptNextPacket = false;
    }

    m_udpSocket->writeDatagram(datagram, QHostAddress(m_targetAddress), m_targetPort);
}

void UavSimulator::onReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        if (!datagram.isValid()) continue;

        ParsedMavlinkMessage msg;
        if (!MavlinkMessageParser::parse(datagram.data(), msg)) {
            continue;
        }

        if (msg.messageId == MAVLINK_MSG_ID_COMMAND_LONG &&
            msg.payload.size() == sizeof(CommandLongPayload)) {

            CommandLongPayload cmd{};
            std::memcpy(&cmd, msg.payload.constData(), sizeof(cmd));

            quint8 result = MAV_RESULT_UNSUPPORTED;

            if (cmd.command == MAV_CMD_NAV_TAKEOFF) {
                if (cmd.param7 >= 10.0f && cmd.param7 <= 500.0f) {
                    m_targetAltitude = cmd.param7;
                    m_flightMode = 4;
                    result = MAV_RESULT_ACCEPTED;
                } else {
                    result = MAV_RESULT_DENIED;
                }
            } else if (cmd.command == MAV_CMD_NAV_LAND) {
                m_targetAltitude = 0.0;
                m_flightMode = 6;
                result = MAV_RESULT_ACCEPTED;
            } else if (cmd.command == MAV_CMD_DO_PAUSE_CONTINUE) {
                m_targetAltitude = m_altitude;
                m_flightMode = 3;
                result = MAV_RESULT_ACCEPTED;
            }

            if (!m_suppressAck) {
                QByteArray ackPacket = MavlinkMessageParser::createCommandAckPacket(
                    m_sequence++, m_systemId, m_componentId, cmd.command, result);
                m_udpSocket->writeDatagram(ackPacket, datagram.senderAddress(), datagram.senderPort());
            }
        }
    }
}