#include "navigation.h"

GPSNavigation::GPSNavigation(Position pos, int satellites, bool valid)
    : m_position(pos), m_satellites(satellites), m_valid(valid) {}

Position GPSNavigation::get_position() const {
    return m_position;
}

bool GPSNavigation::is_valid() const {
    return (m_satellites >= 4) && m_valid;
}

QString GPSNavigation::system_name() const {
    return "GPS";
}

void GPSNavigation::set_position(const Position& pos) {
    m_position = pos;
}

void GPSNavigation::set_satellites(int count) {
    m_satellites = count;
}

void GPSNavigation::set_valid(bool valid) {
    m_valid = valid;
}