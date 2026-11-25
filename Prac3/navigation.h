#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <QString>

class NavigationSystem {
public:
    NavigationSystem();
    virtual ~NavigationSystem() = default;

    bool is_valid() const;
    void set_valid(bool state);

    virtual QString system_name() const = 0;
    virtual double get_altitude(double measured_altitude) const = 0;

private:
    bool valid;
};

class GPSNavigation : public NavigationSystem {
public:
    GPSNavigation();
    QString system_name() const override;
    double get_altitude(double measured_altitude) const override;
};

class InertialNavigation : public NavigationSystem {
public:
    InertialNavigation();
    QString system_name() const override;
    double get_altitude(double measured_altitude) const override;
};

#endif // NAVIGATION_H