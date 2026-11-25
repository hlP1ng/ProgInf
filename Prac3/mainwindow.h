#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <vector>

#include "uav.h"
#include "sensor.h"
#include "navigation.h"
#include "controller.h"
#include "autopilot.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif

    QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:

    void handle_start_clicked();
    void handle_takeoff_clicked();
    void handle_land_clicked();
    void handle_stop_clicked();
    void handle_fault_sensor_clicked();
    void handle_reset_clicked();

    void on_telemetry_labels_updated(const QString& name, double altitude, double error, double control);
    void on_telemetry_chart_updated(const QString& name, double altitude, double error, double control);
    void on_telemetry_table_updated(const QString& name, double altitude, double error, double control);


    void on_position_scene_updated(const QString& name, double x, double y, double heading_deg, double roll);
    void on_mode_changed(const QString& name, const QString& mode);
    void on_fault_detected(const QString& name, const QString& reason);

private:
    void setup_fleet_table();
    void setup_altitude_chart();
    void setup_trajectory_scene();

    static QPainterPath create_quadcopter_icon_path();
    static QPainterPath create_fixed_wing_icon_path();

    Ui::MainWindow* ui;

  
    UAV* vehicle1;
    Gyroscope* gyro1;
    Accelerometer* accel1;
    Barometer* baro1;
    NavigationSystem* nav1;
    FlightController* ctrl1;
    Autopilot* autopilot1;

  
    UAV* vehicle2;
    Gyroscope* gyro2;
    Accelerometer* accel2;
    Barometer* baro2;
    NavigationSystem* nav2;
    FlightController* ctrl2;
    Autopilot* autopilot2;

    std::vector<UAV*> fleet;
    QTimer* cycle_timer;


    QChart* altitude_chart;
    QLineSeries* series_uav1;
    QLineSeries* series_uav2;
    double time_uav1;
    double time_uav2;

    
    QGraphicsScene* scene;
    QGraphicsItemGroup* marker1_heading;
    QGraphicsItemGroup* marker2_heading;
    QGraphicsPathItem* marker1_icon;
    QGraphicsPathItem* marker2_icon;
    QGraphicsPathItem* trajectory1;
    QGraphicsPathItem* trajectory2;
    bool trajectory1_started;
    bool trajectory2_started;


    bool first_fault_reported;
};

#endif // MAINWINDOW_H