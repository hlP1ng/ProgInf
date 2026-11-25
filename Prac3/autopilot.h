#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <QObject>
#include <QString>
#include <cstddef>
#include <utility>
#include <vector>

#include "uav.h"
#include "sensor.h"
#include "navigation.h"
#include "controller.h"

class Autopilot : public QObject {
    Q_OBJECT

public:
    enum class FlightMode {
        Stopped,
        Takeoff,
        Landing,
        Cruise
    };

    Autopilot(const QString& name,
              UAV* vehicle,
              const std::vector<Sensor*>& sensors,
              NavigationSystem* navigation,
              FlightController* controller,
              double initial_target_altitude = 150.0,
              QObject* parent = nullptr);

    QString name() const;
    QString mode_string() const;
    double get_target_altitude() const;

    void command_takeoff();
    void command_land();
    void command_stop();

public slots:
    void run_cycle();

signals:
    void telemetry_updated(const QString& name, double altitude, double error, double control);
    void position_updated(const QString& name, double x, double y, double heading_deg, double roll);
    void fault_detected(const QString& name, const QString& reason);
    void mode_changed(const QString& name, const QString& mode);

private:
    void update_navigation_trajectory(double dt);

    QString vehicle_name;
    UAV* vehicle;
    std::vector<Sensor*> sensors;
    NavigationSystem* navigation;
    FlightController* controller;
    double target_altitude;
    FlightMode mode;
    std::vector<std::pair<double, double>> waypoints;
    std::size_t current_waypoint_index;
};

#endif // AUTOPILOT_H