#ifndef BOSS_HPP
#define BOSS_HPP
#include "Enemy.hpp"

class Boss : public Enemy {
protected:
    bool triggered = false;
    double time_stamp, time;
public:
    Boss(int x, int y);
    void Update(float deltaTime) override;
};
#endif // BOSS_HPP

