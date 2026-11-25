#ifndef UAV_H
#define UAV_H

#include <QString>

class UAV {
public:
    UAV(double initial_altitude = 0.0,
        double initial_battery = 100.0,
        double start_x = 0.0,
        double start_y = 0.0);
    virtual ~UAV() = default;

    virtual QString type() const = 0;
    virtual int motor_count() const = 0;
    virtual double cruise_speed() const = 0;
    virtual void update_altitude(double control, double dt) = 0;

    double altitude() const;
    double battery() const;
    double x() const;
    double y() const;
    double heading() const;
    double roll() const;

    void set_position(double new_x, double new_y);
    void set_heading(double new_heading_rad);
    void set_roll(double new_roll_deg);
    void consume_battery(double amount);

protected:
    void set_altitude_internal(double alt);

private:
    double current_altitude;
    double battery_level;
    double pos_x;
    double pos_y;
    double heading_rad;
    double roll_deg;
};

class Quadcopter : public UAV {
public:
    Quadcopter(double initial_altitude = 0.0,
               double initial_battery = 92.0,
               double start_x = -120.0,
               double start_y = -100.0);

    QString type() const override;
    int motor_count() const override;
    double cruise_speed() const override;
    void update_altitude(double control, double dt) override;

private:
    double vertical_velocity;
};

class FixedWingUAV : public UAV {
public:
    FixedWingUAV(double initial_altitude = 20.0,
                 double initial_battery = 86.0,
                 double start_x = -60.0,
                 double start_y = -100.0);

    QString type() const override;
    int motor_count() const override;
    double cruise_speed() const override;
    void update_altitude(double control, double dt) override;

private:
    double climb_rate;
};

#endif // UAV_H