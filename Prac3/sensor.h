#ifndef SENSOR_H
#define SENSOR_H

#include <QString>

class Sensor {
public:
    explicit Sensor(const QString& name);
    virtual ~Sensor() = default;

    QString name() const;
    bool is_ready() const;
    void set_ready(bool state);

    virtual double read(double true_value) const = 0;

private:
    QString sensor_name;
    bool ready;
};

class Gyroscope : public Sensor {
public:
    explicit Gyroscope(double bias = 1.2);
    double read(double true_value) const override;

private:
    double drift_bias;
};

class Accelerometer : public Sensor {
public:
    explicit Accelerometer(double gravity_ref = 9.81);
    double read(double true_value) const override;

private:
    double gravity;
};

class Barometer : public Sensor {
public:
    explicit Barometer(double sea_level_hpa = 1012.5);
    double read(double true_value) const override;

private:
    double reference_pressure;
};

#endif // SENSOR_H