#ifndef TELEMETRYMODEL_H
#define TELEMETRYMODEL_H

#include <QObject>
#include <QDateTime>
#include <QString>

class TelemetryModel : public QObject
{
    Q_OBJECT
public:
    explicit TelemetryModel(QObject *parent = nullptr);

    void updateTelemetry(quint8 sysId,
                         quint8 compId,
                         double latitude,
                         double longitude,
                         double altitude,
                         const QString &flightMode);

    void updateLink(bool connected,
                    const QString &address,
                    quint16 port);

    quint8 systemId() const;
    quint8 componentId() const;
    double latitude() const;
    double longitude() const;
    double altitude() const;
    QString flightMode() const;
    bool isLinkConnected() const;
    QString lastAddress() const;
    quint16 lastPort() const;
    QDateTime lastUpdate() const;

signals:
    void telemetryChanged(quint8 sysId,
                          quint8 compId,
                          double latitude,
                          double longitude,
                          double altitude,
                          const QString &flightMode);
    void flightModeChanged(const QString &oldMode, const QString &newMode);
    void linkStateChanged(bool connected,
                          const QString &address,
                          quint16 port);

private:
    quint8 m_sysId = 1;
    quint8 m_compId = 1;
    double m_latitude = 0.0;
    double m_longitude = 0.0;
    double m_altitude = 0.0;
    QString m_flightMode = "UNKNOWN";
    bool m_linkConnected = false;
    QString m_lastAddress;
    quint16 m_lastPort = 0;
    QDateTime m_lastUpdate;
};

#endif // TELEMETRYMODEL_H