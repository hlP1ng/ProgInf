#ifndef UAVSIMULATOR_H
#define UAVSIMULATOR_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QString>

class UavSimulator : public QObject
{
    Q_OBJECT
public:
    explicit UavSimulator(QObject *parent = nullptr);

    void start(const QString &targetAddress, quint16 targetPort);
    void stop();
    bool isRunning() const;

    void corruptNextPacket();
    void skipTwoPackets();
    void setSuppressAck(bool suppress);

public slots:
    void sendTelemetry();

private slots:
    void onReadyRead();

private:
    QUdpSocket *m_udpSocket = nullptr;
    QTimer *m_telemetryTimer = nullptr;
    QString m_targetAddress = "127.0.0.1";
    quint16 m_targetPort = 14540;

    double m_altitude = 0.0;
    double m_targetAltitude = 0.0;
    double m_latitude = 55.752220;
    double m_longitude = 37.615560;
    quint16 m_flightMode = 1;

    quint8 m_systemId = 1;
    quint8 m_componentId = 1;
    quint8 m_sequence = 0;
    int m_tickCounter = 0;

    bool m_corruptNextPacket = false;
    bool m_suppressAck = false;
};

#endif // UAVSIMULATOR_H