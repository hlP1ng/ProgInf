#ifndef MAVLINKMANAGER_H
#define MAVLINKMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QDateTime>
#include "mavlinkmessageparser.h"

class TelemetryModel;

class MavlinkManager : public QObject
{
    Q_OBJECT
public:
    explicit MavlinkManager(TelemetryModel *telemetry, QObject *parent = nullptr);

    bool start(quint16 listenPort);
    void stop();
    void setTarget(const QString &address, quint16 port);

    quint64 receivedPackets() const;
    quint64 lostPackets() const;
    quint64 errorPackets() const;

public slots:
    void sendTakeoff(double targetAltitude);
    void sendLand();
    void sendEmergencyStop();

signals:
    void telemetryReceived(quint8 sysId,
                           quint8 compId,
                           double latitude,
                           double longitude,
                           double altitude,
                           const QString &flightMode);
    void linkStateChanged(bool connected,
                          const QString &address,
                          quint16 port);
    void linkRestored();
    void packetReceived(quint64 received,
                        quint64 lost,
                        quint64 errors);
    void commandSent(int command, double param);
    void commandResult(int command,
                       bool accepted,
                       const QString &message);
    void errorOccurred(const QString &message);
    void networkEventLogged(const QString &eventText);

private slots:
    void onReadyRead();
    void checkLinkTimeout();
    void onAckTimeout();

private:
    void processDatagram(const QByteArray &data);
    void handleCommandAck(const CommandAckPayload &ack);
    void sendCommandInternal(quint16 command, float param7, const QString &cmdName);

    QUdpSocket *m_udpSocket = nullptr;
    TelemetryModel *m_telemetry = nullptr;
    QTimer *m_linkTimer = nullptr;
    QTimer *m_ackTimer = nullptr;

    QHostAddress m_targetAddress = QHostAddress::LocalHost;
    quint16 m_targetPort = 14580;

    QString m_lastAddress;
    quint16 m_lastPort = 0;

    quint64 m_receivedPackets = 0;
    quint64 m_lostPackets = 0;
    quint64 m_errorPackets = 0;
    qint64 m_lastSequence = -1;

    quint8 m_systemId = 255;
    quint8 m_componentId = 190;
    quint8 m_targetSystemId = 1;
    quint8 m_targetComponentId = 1;
    quint8 m_txSeq = 0;

    QDateTime m_lastPacketTime;
    bool m_linkConnected = false;
    bool m_wasDisconnectedAfterConnect = false;
    bool m_waitingForAck = false;
    int m_pendingCommand = 0;
};

#endif // MAVLINKMANAGER_H