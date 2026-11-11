#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <QString>

struct Position {
    double latitude;
    double longitude;
    double altitude;
};

// Абстрактный базовый класс системы навигации
class NavigationSystem {
public:
    virtual Position get_position() const = 0;
    virtual bool is_valid() const = 0;
    virtual QString system_name() const = 0;
    virtual ~NavigationSystem() = default;
};

// Спутниковая навигация GPS
class GPSNavigation : public NavigationSystem {
private:
    Position m_position;
    int m_satellites;
    bool m_valid;
public:
    GPSNavigation(Position pos = {55.75, 37.61, 150.0}, int satellites = 8, bool valid = true);

    Position get_position() const override;
    bool is_valid() const override;
    QString system_name() const override;

    void set_position(const Position& pos);
    void set_satellites(int count);
    void set_valid(bool valid);
};

#endif // NAVIGATION_H