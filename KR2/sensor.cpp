#include "sensor.h"
#include <QDebug>

// --- Lidar ---
Lidar::Lidar(double distance, bool ready)
    : m_distance(distance), m_ready(ready) {}

double Lidar::read() { return m_distance; }
bool Lidar::is_ready() const { return m_ready; }
QString Lidar::name() const { return "Lidar"; }

void Lidar::print_reading() {
    if (!m_ready) {
        qDebug().noquote() << "Lidar: sensor is not ready";
        return;
    }
    qDebug().noquote() << QString("Lidar: nearest obstacle=%1 m").arg(read());
}

void Lidar::set_distance(double d) { m_distance = d; }
void Lidar::set_ready(bool r) { m_ready = r; }

// --- Radar ---
Radar::Radar(double distance, bool ready)
    : m_distance(distance), m_ready(ready) {}

double Radar::read() { return m_distance; }
bool Radar::is_ready() const { return m_ready; }
QString Radar::name() const { return "Radar"; }

void Radar::print_reading() {
    if (!m_ready) {
        qDebug().noquote() << "Radar: sensor is not ready";
        return;
    }
    qDebug().noquote() << QString("Radar: nearest object=%1 m").arg(read());
}

void Radar::set_distance(double d) { m_distance = d; }
void Radar::set_ready(bool r) { m_ready = r; }

// --- Camera ---
Camera::Camera(int objects, bool ready)
    : m_objects(objects), m_ready(ready) {}

double Camera::read() { return static_cast<double>(m_objects); }
bool Camera::is_ready() const { return m_ready; }
QString Camera::name() const { return "Camera"; }

void Camera::print_reading() {
    if (!m_ready) {
        qDebug().noquote() << "Camera: sensor is not ready";
        return;
    }
    qDebug().noquote() << QString("Camera: detected objects=%1").arg(static_cast<int>(read()));
}

void Camera::set_objects(int count) { m_objects = count; }
void Camera::set_ready(bool r) { m_ready = r; }

// --- Полиморфная функция проверки препятствия ---
bool obstacle_is_near(Sensor& sensor, double threshold) {
    if (!sensor.is_ready()) {
        qDebug().noquote() << QString("Error: %1 is not ready!").arg(sensor.name());
        return false;
    }
    double val = sensor.read();
    bool is_near = (val <= threshold);
    qDebug().noquote() << QString("Check %1 (value=%2, threshold=%3 m) -> near: %4")
                              .arg(sensor.name())
                              .arg(val)
                              .arg(threshold)
                              .arg(is_near ? "true" : "false");
    return is_near;
}