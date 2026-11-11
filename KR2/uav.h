#ifndef UAV_H
#define UAV_H

#include <QString>

// Абстрактный базовый класс БПЛА (Задание 1, Вариант 3)
class UAV {
public:
    virtual void takeoff() = 0;
    virtual void land() = 0;
    virtual QString type() const = 0;
    virtual QString mission_type() const = 0;
    virtual ~UAV() = default;
};

// БПЛА геодезической разведки / картографии
class SurveyUAV : public UAV {
private:
    QString m_type;
    QString m_mission;
public:
    SurveyUAV();
    void takeoff() override;
    void land() override;
    QString type() const override;
    QString mission_type() const override;
};

// Грузовой БПЛА
class CargoUAV : public UAV {
private:
    QString m_type;
    QString m_mission;
public:
    CargoUAV();
    void takeoff() override;
    void land() override;
    QString type() const override;
    QString mission_type() const override;
};

// Разведывательный БПЛА
class ReconnaissanceUAV : public UAV {
private:
    QString m_type;
    QString m_mission;
public:
    ReconnaissanceUAV();
    void takeoff() override;
    void land() override;
    QString type() const override;
    QString mission_type() const override;
};

// Универсальная полиморфная функция (без if / dynamic_cast)
void execute_test_mission(UAV& vehicle);

#endif // UAV_H