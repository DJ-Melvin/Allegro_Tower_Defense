#ifndef PIGGY_BANK_HPP
#define PIGGY_BANK_HPP
#include "Turret.hpp"

class PiggyTurret: public Turret {
protected:
    double time;
    double time_stamp;
    bool triggered = false;
public:
    static const int Price;
    PiggyTurret(float x, float y);
    void CreateBullet() override;
    void Update(float deltaTime) override;
};
#endif // PIGGY_BANK_HPP

