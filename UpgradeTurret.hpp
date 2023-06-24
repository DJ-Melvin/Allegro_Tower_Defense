#ifndef UPGRADETURRET_HPP
#define UPGRADETURRET_HPP
#include "Turret.hpp"

class UpgradeTurret : public Turret {
public:
    static const int Price;
    UpgradeTurret(float x, float y);
    void CreateBullet() override;
};
#endif // UPGRADETURRET_HPP
