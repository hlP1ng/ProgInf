#include "controller.h"
#include <QDebug>
#include <cmath>

// --- PIDController ---
PIDController::PIDController(double p, double i, double d)
    : kp(p), ki(i), kd(d), integral(0.0), previous_error(0.0) {}

double PIDController::calculate_control(double error, double dt) {
    if (dt <= 0.0) return 0.0;
    integral += error * dt;
    double derivative = (error - previous_error) / dt;
    double output = kp * error + ki * integral + kd * derivative;
    previous_error = error;
    return output;
}

QString PIDController::controller_name() const {
    return "PID";
}

void PIDController::reset() {
    integral = 0.0;
    previous_error = 0.0;
}

// --- SimpleController ---
SimpleController::SimpleController(double coeff)
    : coefficient(coeff) {}

double SimpleController::calculate_control(double error, double /*dt*/) {
    return coefficient * error;
}

QString SimpleController::controller_name() const {
    return "Simple";
}

void SimpleController::set_coefficient(double coeff) {
    coefficient = coeff;
}

// --- FuzzyController ---
FuzzyController::FuzzyController(double s_gain, double m_gain, double l_gain)
    : small_gain(s_gain), medium_gain(m_gain), large_gain(l_gain) {}

double FuzzyController::calculate_control(double error, double /*dt*/) {
    double abs_err = std::abs(error);
    if (abs_err < 5.0) {
        return small_gain * error;
    } else if (abs_err < 25.0) {
        return medium_gain * error;
    } else {
        return large_gain * error;
    }
}

QString FuzzyController::controller_name() const {
    return "Fuzzy";
}

// --- Полиморфная функция тестирования контроллера ---
void test_controller(FlightController& controller, double error, double dt) {
    double output = controller.calculate_control(error, dt);
    qDebug().noquote() << "Controller:" << controller.controller_name();
    qDebug().noquote() << "Error:" << error;
    qDebug().noquote() << "Control output:" << output;
}

// --- Autopilot (Интеграционное задание) ---
Autopilot::Autopilot(UAV* vehicle,
                     const std::vector<Sensor*>& sensors,
                     NavigationSystem* navigation,
                     FlightController* controller)
    : vehicle(vehicle),
    sensors(sensors),
    navigation(navigation),
    controller(controller) {}

void Autopilot::run_cycle() {
    qDebug().noquote() << "Autopilot cycle started";

    // 1. Проверка наличия БПЛА
    if (vehicle == nullptr) {
        qDebug().noquote() << "Error: vehicle is not set";
        return;
    }

    // Проверка наличия полётного контроллера (Сценарий 4)
    if (controller == nullptr) {
        qDebug().noquote() << "Error: flight controller is not set";
        return;
    }

    // 2. Проверка сенсоров (указатель, готовность, чтение значения)
    if (sensors.empty()) {
        qDebug().noquote() << "Autopilot cycle aborted / Reason: sensor failure";
        return;
    }

    for (Sensor* s : sensors) {
        if (s == nullptr || !s->is_ready()) {
            qDebug().noquote() << "Autopilot cycle aborted / Reason: sensor failure";
            return;
        }
        (void)s->read(); // Контрольное чтение значения
    }
    qDebug().noquote() << "All sensors are ready";

    // 3. Проверка навигации
    if (navigation == nullptr || !navigation->is_valid()) {
        qDebug().noquote() << "Navigation data invalid";
        return;
    }

    // 4. Получение позиции и проверка корректности координат (Сценарий 5)
    Position position = navigation->get_position();
    if (position.altitude < 0.0) {
        qDebug().noquote() << "Error: invalid navigation position";
        return;
    }

    qDebug().noquote() << "Navigation:" << navigation->system_name();
    qDebug().noquote() << QString("Current altitude: %1 m").arg(position.altitude);

    // 5. Вычисление ошибки по высоте (target_altitude = 200.0)
    const double target_altitude = 200.0;
    double error = target_altitude - position.altitude;
    qDebug().noquote() << QString("Target altitude: %1 m").arg(target_altitude);
    qDebug().noquote() << QString("Altitude error: %1 m").arg(error);

    // 6. Вычисление управляющего воздействия
    double control = controller->calculate_control(error, 0.1);

    // 7. Вывод результата
    qDebug().noquote() << QString("Controller output: %1").arg(control);
    qDebug().noquote() << "Autopilot cycle completed";
}