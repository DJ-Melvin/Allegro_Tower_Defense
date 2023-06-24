#ifndef CIRCLEBULLET_HPP
#define CIRCLEBULLET_HPP
#include "Bullet.hpp"
#include "CircleTurret.hpp"
class Enemy;
class Turret;
namespace Engine {
    struct Point;
}  // namespace Engine

class CircleBullet : public Bullet {
public:
    CircleTurret* real_parent;
    int omega = 3;
    int radius = 150;
    float angle;
    explicit CircleBullet(Engine::Point position, float angle, Turret* parent, CircleTurret* real_parent);
    void OnExplode(Enemy* enemy) override;
    void Update(float deltaTime) override;
};
#endif // CIRCLEBULLET_HPP
