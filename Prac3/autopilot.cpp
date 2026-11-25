#include "autopilot.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr double kPi = 3.14159265358979323846;

double normalize_angle(double angle) {
    while (angle > kPi) {
        angle -= 2.0 * kPi;
    }
    while (angle < -kPi) {
        angle += 2.0 * kPi;
    }
    return angle;
}
} // namespace

Autopilot::Autopilot(const QString& name,
                     UAV* vehicle,
                     const std::vector<Sensor*>& sensors,
                     NavigationSystem* navigation,
                     FlightController* controller,
                     double initial_target_altitude,
                     QObject* parent)
    : QObject(parent),
    vehicle_name(name),
    vehicle(vehicle),
    sensors(sensors),
    navigation(navigation),
    controller(controller),
    target_altitude(initial_target_altitude),
    mode(FlightMode::Cruise),
    waypoints({
        {-130.0, -110.0},
        { 130.0, -110.0},
        { 130.0,  110.0},
        {-130.0,  110.0}
    }),
    current_waypoint_index(1) {}

QString Autopilot::name() const {
    return vehicle_name;
}

QString Autopilot::mode_string() const {
    switch (mode) {
    case FlightMode::Stopped:
        return "Stopped";
    case FlightMode::Takeoff:
        return "Takeoff (200 m)";
    case FlightMode::Landing:
        return "Landing (0 m)";
    case FlightMode::Cruise:
        return "Cruise (150 m)";
    }
    return "Unknown";
}

double Autopilot::get_target_altitude() const {
    return target_altitude;
}

void Autopilot::command_takeoff() {
    mode = FlightMode::Takeoff;
    target_altitude = 200.0;
    if (controller != nullptr) {
        controller->reset();
    }
    emit mode_changed(vehicle_name, mode_string());
}

void Autopilot::command_land() {
    mode = FlightMode::Landing;
    target_altitude = 0.0;
    if (controller != nullptr) {
        controller->reset();
    }
    emit mode_changed(vehicle_name, mode_string());
}

void Autopilot::command_stop() {
    mode = FlightMode::Stopped;
    if (vehicle != nullptr) {
        target_altitude = vehicle->altitude();
        vehicle->set_roll(0.0);
    }
    emit mode_changed(vehicle_name, mode_string());
}

void Autopilot::run_cycle() {
    if (mode == FlightMode::Stopped) {
        return;
    }

    if (vehicle == nullptr) {
        emit fault_detected(vehicle_name, "vehicle is not set");
        return;
    }

    for (Sensor* sensor : sensors) {
        if (sensor == nullptr || !sensor->is_ready()) {
            emit fault_detected(vehicle_name,
                                QString("sensor failure: ") + (sensor ? sensor->name() : "unknown"));
            return;
        }
    }

    if (navigation == nullptr || !navigation->is_valid()) {
        emit fault_detected(vehicle_name, "navigation data invalid");
        return;
    }

    if (controller == nullptr) {
        emit fault_detected(vehicle_name, "controller is not set");
        return;
    }

    constexpr double dt = 0.1; // 10 Гц

    double sensed_altitude = vehicle->altitude();
    for (Sensor* sensor : sensors) {
        if (sensor->name() == "Barometer") {
            sensed_altitude = sensor->read(vehicle->altitude());
        } else {
            sensor->read(vehicle->roll());
        }
    }

    const double nav_altitude = navigation->get_altitude(sensed_altitude);
    const double error = target_altitude - nav_altitude;
    const double control = controller->compute(target_altitude, nav_altitude, dt);

    vehicle->update_altitude(control, dt);
    update_navigation_trajectory(dt);

    const double heading_deg = vehicle->heading() * 180.0 / kPi;

    emit telemetry_updated(vehicle_name, vehicle->altitude(), error, control);
    emit position_updated(vehicle_name, vehicle->x(), vehicle->y(), heading_deg, vehicle->roll());
}

void Autopilot::update_navigation_trajectory(double dt) {
 
    if (mode == FlightMode::Landing && vehicle->altitude() <= 0.05) {
        vehicle->set_roll(0.0);
        return;
    }

    const auto& wp = waypoints[current_waypoint_index];
    const double dx = wp.first - vehicle->x();
    const double dy = wp.second - vehicle->y();
    const double distance = std::hypot(dx, dy);

    if (distance < 10.0) {
        current_waypoint_index = (current_waypoint_index + 1) % waypoints.size();
    }

    const auto& active_wp = waypoints[current_waypoint_index];
    const double desired_heading = std::atan2(active_wp.second - vehicle->y(),
                                              active_wp.first - vehicle->x());
    const double heading_error = normalize_angle(desired_heading - vehicle->heading());

    const double max_turn_rate = 1.6; // рад/с
    const double turn_rate = std::clamp(heading_error * 2.2, -max_turn_rate, max_turn_rate);
    const double new_heading = normalize_angle(vehicle->heading() + turn_rate * dt);

   
    const double bank_roll_deg = std::clamp((turn_rate / max_turn_rate) * 35.0, -35.0, 35.0);

    const double speed = vehicle->cruise_speed();
    const double next_x = vehicle->x() + std::cos(new_heading) * speed * dt;
    const double next_y = vehicle->y() + std::sin(new_heading) * speed * dt;

    vehicle->set_heading(new_heading);
    vehicle->set_roll(bank_roll_deg);
    vehicle->set_position(next_x, next_y);
}