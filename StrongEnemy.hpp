#ifndef STRONGENEMY_HPP
#define STRONGENEMY_HPP
#include "Enemy.hpp"

class StrongEnemy : public Enemy {
public:
    StrongEnemy(int x, int y);
    void OnExplode() override;
};
#endif // STRONGENEMY_HPP
