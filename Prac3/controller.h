#ifndef CONTROLLER_H
#define CONTROLLER_H

class FlightController {
public:
    virtual ~FlightController() = default;
    virtual double compute(double target, double current, double dt) = 0;
    virtual void reset() = 0;
};

class PIDController : public FlightController {
public:
    PIDController(double p, double i, double d);

    double compute(double target, double current, double dt) override;
    void reset() override;

private:
    double kp;
    double ki;
    double kd;
    double integral;
    double prev_error;
    bool first_sample;
};

#endif // CONTROLLER_H