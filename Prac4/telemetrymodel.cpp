#include "telemetrymodel.h"

TelemetryModel::TelemetryModel(QObject *parent)
    : QObject(parent)
{
}

void TelemetryModel::updateTelemetry(quint8 sysId,
                                     quint8 compId,
                                     double latitude,
                                     double longitude,
                                     double altitude,
                                     const QString &flightMode)
{
    if (m_flightMode != flightMode && m_flightMode != "UNKNOWN") {
        emit flightModeChanged(m_flightMode, flightMode);
    }

    m_sysId = sysId;
    m_compId = compId;
    m_latitude = latitude;
    m_longitude = longitude;
    m_altitude = altitude;
    m_flightMode = flightMode;
    m_lastUpdate = QDateTime::currentDateTime();

    emit telemetryChanged(m_sysId, m_compId, m_latitude, m_longitude, m_altitude, m_flightMode);
}

void TelemetryModel::updateLink(bool connected, const QString &address, quint16 port)
{
    m_linkConnected = connected;
    if (!address.isEmpty()) {
        m_lastAddress = address;
    }
    if (port != 0) {
        m_lastPort = port;
    }
    if (connected) {
        m_lastUpdate = QDateTime::currentDateTime();
    }

    emit linkStateChanged(m_linkConnected, m_lastAddress, m_lastPort);
}

quint8 TelemetryModel::systemId() const { return m_sysId; }
quint8 TelemetryModel::componentId() const { return m_compId; }
double TelemetryModel::latitude() const { return m_latitude; }
double TelemetryModel::longitude() const { return m_longitude; }
double TelemetryModel::altitude() const { return m_altitude; }
QString TelemetryModel::flightMode() const { return m_flightMode; }
bool TelemetryModel::isLinkConnected() const { return m_linkConnected; }
QString TelemetryModel::lastAddress() const { return m_lastAddress; }
quint16 TelemetryModel::lastPort() const { return m_lastPort; }
QDateTime TelemetryModel::lastUpdate() const { return m_lastUpdate; }