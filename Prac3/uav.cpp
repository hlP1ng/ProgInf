#include "uav.h"
#include <algorithm>

UAV::UAV(double initial_altitude,
         double initial_battery,
         double start_x,
         double start_y)
    : current_altitude(initial_altitude),
    battery_level(initial_battery),
    pos_x(start_x),
    pos_y(start_y),
    heading_rad(0.0),
    roll_deg(0.0) {}

double UAV::altitude() const {
    return current_altitude;
}

double UAV::battery() const {
    return battery_level;
}

double UAV::x() const {
    return pos_x;
}

double UAV::y() const {
    return pos_y;
}

double UAV::heading() const {
    return heading_rad;
}

double UAV::roll() const {
    return roll_deg;
}

void UAV::set_position(double new_x, double new_y) {
    pos_x = new_x;
    pos_y = new_y;
}

void UAV::set_heading(double new_heading_rad) {
    heading_rad = new_heading_rad;
}

void UAV::set_roll(double new_roll_deg) {
    roll_deg = std::clamp(new_roll_deg, -45.0, 45.0);
}

void UAV::consume_battery(double amount) {
    battery_level = std::max(0.0, battery_level - amount);
}

void UAV::set_altitude_internal(double alt) {
    current_altitude = std::max(0.0, alt);
}

Quadcopter::Quadcopter(double initial_altitude,
                       double initial_battery,
                       double start_x,
                       double start_y)
    : UAV(initial_altitude, initial_battery, start_x, start_y),
    vertical_velocity(0.0) {}

QString Quadcopter::type() const {
    return "Quadcopter";
}

int Quadcopter::motor_count() const {
    return 4;
}

double Quadcopter::cruise_speed() const {
    return 24.0;
}

void Quadcopter::update_altitude(double control, double dt) {
    const double clamped_ctrl = std::clamp(control, -40.0, 40.0);
    vertical_velocity += (clamped_ctrl - 0.8 * vertical_velocity) * dt;
    set_altitude_internal(altitude() + vertical_velocity * dt);

    if (altitude() <= 0.0 && vertical_velocity < 0.0) {
        vertical_velocity = 0.0;
    }

    if (altitude() > 0.05) {
        consume_battery(0.015 * dt);
    }
}

FixedWingUAV::FixedWingUAV(double initial_altitude,
                           double initial_battery,
                           double start_x,
                           double start_y)
    : UAV(initial_altitude, initial_battery, start_x, start_y),
    climb_rate(0.0) {}

QString FixedWingUAV::type() const {
    return "FixedWingUAV";
}

int FixedWingUAV::motor_count() const {
    return 1;
}

double FixedWingUAV::cruise_speed() const {
    return 36.0;
}

void FixedWingUAV::update_altitude(double control, double dt) {
    const double clamped_ctrl = std::clamp(control, -35.0, 35.0);
    climb_rate += (0.85 * clamped_ctrl - 0.65 * climb_rate) * dt;
    set_altitude_internal(altitude() + climb_rate * dt);

    if (altitude() <= 0.0 && climb_rate < 0.0) {
        climb_rate = 0.0;
    }

    if (altitude() > 0.05) {
        consume_battery(0.010 * dt);
    }
}