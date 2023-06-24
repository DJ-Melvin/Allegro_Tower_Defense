#include <string>
#include <random>

#include "StrongEnemy.hpp"
#include "PlayScene.hpp"
#include "Enemy.hpp"
#include "GameEngine.hpp"
#include "BlueNormalEnemy.hpp"
#include "ExplosionEffect.hpp"

StrongEnemy::StrongEnemy(int x, int y) : Enemy("play/dice-2.png", x, y, 25, 55, 100, 5) {}
void StrongEnemy::OnExplode() {
    Enemy* enemy;
    getPlayScene()->EffectGroup->AddNewObject(new ExplosionEffect(Position.x, Position.y));
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> distId(1, 3);
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, 20);
    getPlayScene()->EnemyGroup->AddNewObject(enemy = new BlueNormalEnemy(Position.x, Position.y));
    enemy->UpdatePath(getPlayScene()->mapDistance);
    enemy->Update(0);
}
