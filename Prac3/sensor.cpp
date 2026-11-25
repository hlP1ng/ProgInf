#include "sensor.h"

Sensor::Sensor(const QString& name)
    : sensor_name(name),
    ready(true) {}

QString Sensor::name() const {
    return sensor_name;
}

bool Sensor::is_ready() const {
    return ready;
}

void Sensor::set_ready(bool state) {
    ready = state;
}

Gyroscope::Gyroscope(double bias)
    : Sensor("Gyroscope"),
    drift_bias(bias) {}

double Gyroscope::read(double true_value) const {
    return true_value + drift_bias * 0.01;
}

Accelerometer::Accelerometer(double gravity_ref)
    : Sensor("Accelerometer"),
    gravity(gravity_ref) {}

double Accelerometer::read(double true_value) const {
    return true_value * (gravity / 9.81);
}

Barometer::Barometer(double sea_level_hpa)
    : Sensor("Barometer"),
    reference_pressure(sea_level_hpa) {}

double Barometer::read(double true_value) const {
    const double pressure_factor = reference_pressure / 1012.5;
    return true_value * pressure_factor;
}