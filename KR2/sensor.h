#ifndef SENSOR_H
#define SENSOR_H

#include <QString>
#include <vector>

// Абстрактный базовый класс сенсора (Задание 2, Вариант 3)
class Sensor {
public:
    virtual double read() = 0;
    virtual bool is_ready() const = 0;
    virtual QString name() const = 0;
    virtual void print_reading() = 0; // Для полиморфного вывода без if/dynamic_cast
    virtual ~Sensor() = default;
};

// Лидар (расстояние до ближайшего препятствия, м)
class Lidar : public Sensor {
private:
    double m_distance;
    bool m_ready;
public:
    explicit Lidar(double distance = 12.5, bool ready = true);
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    void print_reading() override;

    void set_distance(double d);
    void set_ready(bool r);
};

// Радар (расстояние до объекта, м)
class Radar : public Sensor {
private:
    double m_distance;
    bool m_ready;
public:
    explicit Radar(double distance = 18.2, bool ready = true);
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    void print_reading() override;

    void set_distance(double d);
    void set_ready(bool r);
};

// Камера (число объектов в кадре)
class Camera : public Sensor {
private:
    int m_objects;
    bool m_ready;
public:
    explicit Camera(int objects = 3, bool ready = true);
    double read() override;
    bool is_ready() const override;
    QString name() const override;
    void print_reading() override;

    void set_objects(int count);
    void set_ready(bool r);
};

// Универсальная функция проверки близости препятствия
bool obstacle_is_near(Sensor& sensor, double threshold);

#endif // SENSOR_H