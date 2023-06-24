#ifndef TURRETTWO_HPP
#define TURRETTWO_HPP
#include "Turret.hpp"

class TurretTwo : public Turret {
public:
    static const int Price;
    TurretTwo(float x, float y);
    void CreateBullet() override;
};
#endif // TURRETTWO_HPP
