#include "controller.h"
#include <algorithm>

PIDController::PIDController(double p, double i, double d)
    : kp(p),
    ki(i),
    kd(d),
    integral(0.0),
    prev_error(0.0),
    first_sample(true) {}

double PIDController::compute(double target, double current, double dt) {
    if (dt <= 0.0) {
        return 0.0;
    }

    const double error = target - current;
    integral = std::clamp(integral + error * dt, -100.0, 100.0);

    double derivative = 0.0;
    if (!first_sample) {
        derivative = (error - prev_error) / dt;
    }
    first_sample = false;
    prev_error = error;

    return kp * error + ki * integral + kd * derivative;
}

void PIDController::reset() {
    integral = 0.0;
    prev_error = 0.0;
    first_sample = true;
}