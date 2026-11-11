#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QString>
#include <vector>
#include "uav.h"
#include "sensor.h"
#include "navigation.h"

// Абстрактный класс полётного контроллера (Задание 3, Вариант 3)
class FlightController {
public:
    virtual double calculate_control(double error, double dt) = 0;
    virtual QString controller_name() const = 0;
    virtual ~FlightController() = default;
};

// ПИД-регулятор
class PIDController : public FlightController {
private:
    double kp;
    double ki;
    double kd;
    double integral;
    double previous_error;
public:
    // Коэффициенты по умолчанию дают ровно 1.87 при error=2.5, dt=0.1 (как в примере из задания)
    PIDController(double p = 0.5, double i = 0.08, double d = 0.024);
    double calculate_control(double error, double dt) override;
    QString controller_name() const override;
    void reset();
};

// Простой пропорциональный регулятор
class SimpleController : public FlightController {
private:
    double coefficient;
public:
    // При coefficient = 0.5 и ошибке 50 м выдаёт ровно 25 (как в эталоне Autopilot)
    explicit SimpleController(double coeff = 0.5);
    double calculate_control(double error, double dt) override;
    QString controller_name() const override;
    void set_coefficient(double coeff);
};

// Нечёткий (Fuzzy) контроллер
class FuzzyController : public FlightController {
private:
    double small_gain;
    double medium_gain;
    double large_gain;
public:
    FuzzyController(double s_gain = 0.3, double m_gain = 0.6, double l_gain = 0.9);
    double calculate_control(double error, double dt) override;
    QString controller_name() const override;
};

// Полиморфная функция тестирования любого контроллера
void test_controller(FlightController& controller, double error, double dt);

// Интеграционное задание: Класс Autopilot
class Autopilot {
private:
    UAV* vehicle;
    std::vector<Sensor*> sensors;
    NavigationSystem* navigation;
    FlightController* controller;
public:
    Autopilot(UAV* vehicle,
              const std::vector<Sensor*>& sensors,
              NavigationSystem* navigation,
              FlightController* controller);
    void run_cycle();
};

#endif // CONTROLLER_H