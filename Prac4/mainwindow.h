#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif

    QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class TelemetryModel;
class MavlinkManager;
class UavSimulator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onTelemetryChanged(quint8 sysId,
                            quint8 compId,
                            double latitude,
                            double longitude,
                            double altitude,
                            const QString &flightMode);
    void onFlightModeChanged(const QString &oldMode, const QString &newMode);
    void onLinkStateChanged(bool connected,
                            const QString &address,
                            quint16 port);
    void onLinkRestored();
    void onPacketReceived(quint64 received, quint64 lost, quint64 errors);
    void onCommandResult(int command, bool accepted, const QString &message);
    void onMavlinkError(const QString &message);
    void appendNetworkEvent(const QString &eventText);

    void onBtnStartLinkClicked();
    void onBtnStopLinkClicked();
    void onBtnTakeoffClicked();
    void onBtnLandClicked();
    void onBtnStopCommandClicked();

    void onBtnStartUavClicked();
    void onBtnDisconnectUavClicked();
    void onBtnCorruptPacketClicked();
    void onBtnSkipPacketsClicked();
    void onChkSuppressAckToggled(bool checked);

private:
    void setupChart();
    void setupTable();
    void setCommandButtonsEnabled(bool enabled);

    Ui::MainWindow *ui = nullptr;
    TelemetryModel *m_telemetryModel = nullptr;
    MavlinkManager *m_mavlinkManager = nullptr;
    UavSimulator *m_uavSimulator = nullptr;

    QLineSeries *m_altitudeSeries = nullptr;
    QChart *m_chart = nullptr;
    QValueAxis *m_axisX = nullptr;
    QValueAxis *m_axisY = nullptr;
    double m_timeSeconds = 0.0;

    QDateTime m_lastTableAddTime;
};

#endif // MAINWINDOW_H