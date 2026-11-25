#include "mavlinkmanager.h"
#include "telemetrymodel.h"
#include <QNetworkDatagram>
#include <cstring>

MavlinkManager::MavlinkManager(TelemetryModel *telemetry, QObject *parent)
    : QObject(parent)
    , m_telemetry(telemetry)
{
    m_linkTimer = new QTimer(this);
    m_linkTimer->setInterval(1000);
    connect(m_linkTimer, &QTimer::timeout, this, &MavlinkManager::checkLinkTimeout);

    m_ackTimer = new QTimer(this);
    m_ackTimer->setSingleShot(true);
    m_ackTimer->setInterval(3000);
    connect(m_ackTimer, &QTimer::timeout, this, &MavlinkManager::onAckTimeout);
}

bool MavlinkManager::start(quint16 listenPort)
{
    if (m_udpSocket) {
        m_udpSocket->close();
        m_udpSocket->deleteLater();
        m_udpSocket = nullptr;
    }

    m_udpSocket = new QUdpSocket(this);
    if (!m_udpSocket->bind(QHostAddress::AnyIPv4, listenPort)) {
        emit errorOccurred(QString("Не удалось открыть UDP-порт %1: %2")
                               .arg(listenPort)
                               .arg(m_udpSocket->errorString()));
        return false;
    }

    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &MavlinkManager::onReadyRead);

    m_linkConnected = false;
    m_wasDisconnectedAfterConnect = false;
    m_lastSequence = -1;
    m_lastPacketTime = QDateTime();
    m_linkTimer->start();

    emit networkEventLogged(QString("UDP-порт %1 открыт (ожидание пакетов)").arg(listenPort));
    emit linkStateChanged(false, QString(), 0);
    return true;
}

void MavlinkManager::stop()
{
    m_linkTimer->stop();
    m_ackTimer->stop();
    m_waitingForAck = false;

    if (m_udpSocket) {
        m_udpSocket->close();
        m_udpSocket->deleteLater();
        m_udpSocket = nullptr;
    }

    m_linkConnected = false;
    emit networkEventLogged("UDP-канал закрыт оператором");
    emit linkStateChanged(false, QString(), 0);
}

void MavlinkManager::setTarget(const QString &address, quint16 port)
{
    m_targetAddress = QHostAddress(address);
    m_targetPort = port;
}

quint64 MavlinkManager::receivedPackets() const { return m_receivedPackets; }
quint64 MavlinkManager::lostPackets() const { return m_lostPackets; }
quint64 MavlinkManager::errorPackets() const { return m_errorPackets; }

void MavlinkManager::onReadyRead()
{
    while (m_udpSocket && m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        if (!datagram.isValid()) {
            continue;
        }

        m_lastAddress = datagram.senderAddress().toString();
        m_lastPort = datagram.senderPort();
        m_targetAddress = datagram.senderAddress();
        m_targetPort = datagram.senderPort();

        processDatagram(datagram.data());
    }
}

void MavlinkManager::processDatagram(const QByteArray &data)
{
    QString errorReason;
    ParsedMavlinkMessage msg;
    if (!MavlinkMessageParser::parse(data, msg, &errorReason)) {
        m_errorPackets++;
        emit packetReceived(m_receivedPackets, m_lostPackets, m_errorPackets);
        emit errorOccurred(errorReason);
        emit networkEventLogged("Ошибка пакета: " + errorReason);
        return;
    }

    m_receivedPackets++;
    m_lastPacketTime = QDateTime::currentDateTime();

    if (!m_linkConnected) {
        m_linkConnected = true;
        emit networkEventLogged(QString("Принят пакет от %1:%2").arg(m_lastAddress).arg(m_lastPort));
        if (m_wasDisconnectedAfterConnect) {
            emit networkEventLogged("Связь восстановлена");
            emit linkRestored();
        }
        emit linkStateChanged(true, m_lastAddress, m_lastPort);
    }

    const quint8 sequence = msg.sequence;
    if (m_lastSequence >= 0) {
        const qint64 expected = (m_lastSequence + 1) % 256;
        if (sequence != expected) {
            const qint64 lost = (sequence - expected + 256) % 256;
            if (lost > 0 && lost < 100) {
                m_lostPackets += static_cast<quint64>(lost);
                emit networkEventLogged(QString("Потеря пакетов: %1").arg(lost));
            }
        }
    }
    m_lastSequence = sequence;

    emit packetReceived(m_receivedPackets, m_lostPackets, m_errorPackets);

    m_targetSystemId = msg.systemId;
    m_targetComponentId = msg.componentId;

    if (msg.messageId == MAVLINK_MSG_ID_HEARTBEAT) {
        if (msg.payload.size() == sizeof(HeartbeatPayload)) {
            emit networkEventLogged(QString("MAVLink HEARTBEAT, sysid=%1, compid=%2")
                                        .arg(msg.systemId)
                                        .arg(msg.componentId));
        }
    } else if (msg.messageId == MAVLINK_MSG_ID_GLOBAL_POSITION_INT) {
        if (msg.payload.size() != sizeof(SimpleTelemetryPayload)) {
            emit errorOccurred("Неверный размер телеметрического пакета");
            return;
        }
        SimpleTelemetryPayload telemetry{};
        std::memcpy(&telemetry, msg.payload.constData(), sizeof(telemetry));

        const double latitude  = telemetry.latitudeE7 / 1e7;
        const double longitude = telemetry.longitudeE7 / 1e7;
        const double altitude  = telemetry.altitudeMm / 1000.0;
        const QString flightMode = MavlinkMessageParser::flightModeToString(telemetry.flightMode);

        emit telemetryReceived(msg.systemId, msg.componentId, latitude, longitude, altitude, flightMode);
    } else if (msg.messageId == MAVLINK_MSG_ID_COMMAND_ACK) {
        if (msg.payload.size() == sizeof(CommandAckPayload)) {
            CommandAckPayload ack{};
            std::memcpy(&ack, msg.payload.constData(), sizeof(ack));
            handleCommandAck(ack);
        }
    }
}

void MavlinkManager::sendCommandInternal(quint16 command, float param7, const QString &cmdName)
{
    if (!m_udpSocket) {
        emit errorOccurred("UDP-порт не открыт");
        return;
    }
    if (m_targetAddress.isNull() || m_targetPort == 0) {
        emit errorOccurred("Адрес БПЛА не задан");
        return;
    }
    if (!m_telemetry || !m_telemetry->isLinkConnected()) {
        emit errorOccurred("Невозможно отправить команду: нет связи");
        return;
    }

    const QByteArray datagram = MavlinkMessageParser::createCommandLongPacket(
        m_txSeq++,
        m_systemId,
        m_componentId,
        m_targetSystemId,
        m_targetComponentId,
        command,
        param7
        );

    const qint64 sent = m_udpSocket->writeDatagram(datagram, m_targetAddress, m_targetPort);
    if (sent == -1) {
        emit errorOccurred("Ошибка отправки команды " + cmdName);
        return;
    }

    m_waitingForAck = true;
    m_pendingCommand = command;
    m_ackTimer->start();

    emit networkEventLogged(QString("Отправлена команда %1 (ID=%2)").arg(cmdName).arg(command));
    emit commandSent(static_cast<int>(command), param7);
}

void MavlinkManager::sendTakeoff(double targetAltitude)
{
    sendCommandInternal(MAV_CMD_NAV_TAKEOFF,
                        static_cast<float>(targetAltitude),
                        QString("MAV_CMD_NAV_TAKEOFF (%1 м)").arg(targetAltitude, 0, 'f', 0));
}

void MavlinkManager::sendLand()
{
    sendCommandInternal(MAV_CMD_NAV_LAND, 0.0f, "MAV_CMD_NAV_LAND");
}

void MavlinkManager::sendEmergencyStop()
{
    sendCommandInternal(MAV_CMD_DO_PAUSE_CONTINUE, 0.0f, "MAV_CMD_DO_PAUSE_CONTINUE (СТОП)");
}

void MavlinkManager::handleCommandAck(const CommandAckPayload &ack)
{
    if (m_waitingForAck && ack.command == m_pendingCommand) {
        m_waitingForAck = false;
        m_ackTimer->stop();
    }

    QString resultText;
    switch (ack.result) {
    case MAV_RESULT_ACCEPTED:
        resultText = "Команда принята";
        break;
    case MAV_RESULT_TEMPORARILY_REJECTED:
        resultText = "Команда временно отклонена";
        break;
    case MAV_RESULT_DENIED:
        resultText = "Команда отклонена";
        break;
    case MAV_RESULT_UNSUPPORTED:
        resultText = "Команда не поддерживается";
        break;
    case MAV_RESULT_FAILED:
        resultText = "Ошибка выполнения команды";
        break;
    case MAV_RESULT_IN_PROGRESS:
        resultText = "Команда выполняется";
        break;
    default:
        resultText = "Неизвестный результат";
        break;
    }

    const bool accepted = (ack.result == MAV_RESULT_ACCEPTED || ack.result == MAV_RESULT_IN_PROGRESS);
    emit networkEventLogged(QString("Получен COMMAND_ACK (cmd=%1): %2").arg(ack.command).arg(resultText));
    emit commandResult(static_cast<int>(ack.command), accepted, resultText);
}

void MavlinkManager::checkLinkTimeout()
{
    if (!m_lastPacketTime.isValid()) {
        return;
    }

    const qint64 elapsedMs = m_lastPacketTime.msecsTo(QDateTime::currentDateTime());
    if (elapsedMs > 3000 && m_linkConnected) {
        m_linkConnected = false;
        m_wasDisconnectedAfterConnect = true;
        emit networkEventLogged("Связь потеряна");
        emit linkStateChanged(false, m_lastAddress, m_lastPort);
        emit errorOccurred("Связь с БПЛА потеряна");
    }
}

void MavlinkManager::onAckTimeout()
{
    if (!m_waitingForAck) {
        return;
    }
    m_waitingForAck = false;
    emit networkEventLogged(QString("Таймаут ожидания COMMAND_ACK для команды %1").arg(m_pendingCommand));
    emit commandResult(m_pendingCommand, false, "Подтверждение не получено");
}