#ifndef CIRCLETURRET_HPP
#define CIRCLETURRET_HPP
#include "Turret.hpp"

class CircleTurret : public Turret {
public:
    Bullet* Child[4] = {nullptr};
    int radius = 200;
    static const int Price;
    int bullet_num = 0;
    CircleTurret(float x, float y);
    void CreateBullet() override;
    ~CircleTurret();
    void Update(float deltaTime) override;
};
#endif // CIRCLETURRET_HPP

