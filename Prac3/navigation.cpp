#include "navigation.h"

NavigationSystem::NavigationSystem()
    : valid(true) {}

bool NavigationSystem::is_valid() const {
    return valid;
}

void NavigationSystem::set_valid(bool state) {
    valid = state;
}

GPSNavigation::GPSNavigation() = default;

QString GPSNavigation::system_name() const {
    return "GPS";
}

double GPSNavigation::get_altitude(double measured_altitude) const {
    return measured_altitude;
}

InertialNavigation::InertialNavigation() = default;

QString InertialNavigation::system_name() const {
    return "INS";
}

double InertialNavigation::get_altitude(double measured_altitude) const {
    return measured_altitude;
}