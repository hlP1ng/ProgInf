#include "uav.h"
#include <QDebug>

// --- SurveyUAV ---
SurveyUAV::SurveyUAV()
    : m_type("Survey UAV"), m_mission("terrain mapping") {}

void SurveyUAV::takeoff() {
    qDebug().noquote() << m_type + ": catapult launch takeoff";
}

void SurveyUAV::land() {
    qDebug().noquote() << m_type + ": parachute landing";
}

QString SurveyUAV::type() const { return m_type; }
QString SurveyUAV::mission_type() const { return m_mission; }

// --- CargoUAV ---
CargoUAV::CargoUAV()
    : m_type("Cargo UAV"), m_mission("payload transportation") {}

void CargoUAV::takeoff() {
    qDebug().noquote() << m_type + ": heavy vertical takeoff";
}

void CargoUAV::land() {
    qDebug().noquote() << m_type + ": precision vertical landing";
}

QString CargoUAV::type() const { return m_type; }
QString CargoUAV::mission_type() const { return m_mission; }

// --- ReconnaissanceUAV ---
ReconnaissanceUAV::ReconnaissanceUAV()
    : m_type("Reconnaissance UAV"), m_mission("observation and monitoring") {}

void ReconnaissanceUAV::takeoff() {
    qDebug().noquote() << m_type + ": silent runway takeoff";
}

void ReconnaissanceUAV::land() {
    qDebug().noquote() << m_type + ": gliding landing";
}

QString ReconnaissanceUAV::type() const { return m_type; }
QString ReconnaissanceUAV::mission_type() const { return m_mission; }

// --- Полиморфная функция испытания миссии ---
void execute_test_mission(UAV& vehicle) {
    // Строго ожидаемый формат вывода из задания:
    qDebug().noquote() << QString("%1: %2").arg(vehicle.type(), vehicle.mission_type());
    vehicle.takeoff();
    vehicle.land();
}