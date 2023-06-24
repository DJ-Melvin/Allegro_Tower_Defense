#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>
#include <random>
#include <string>
#include <vector>

#include "AudioHelper.hpp"
#include "Bullet.hpp"
#include "DirtyEffect.hpp"
#include "Enemy.hpp"
#include "ExplosionEffect.hpp"
#include "GameEngine.hpp"
#include "Group.hpp"
#include "IScene.hpp"
#include "LOG.hpp"
#include "PlayScene.hpp"
#include "Turret.hpp"
#include "Boss.hpp"
#include "BlueNormalEnemy.hpp"

Boss::Boss(int x, int y) : Enemy("play/enemy-6.png", x, y, 25, 45, 300, 5) {

}
void Boss::Update(float deltaTime) {
    // Slow effect.
	SlowEffect(deltaTime);
	// Pre-calculate the velocity.
	float remainSpeed = speed * deltaTime;
	while (remainSpeed != 0) {
		if (path.empty()) {
			// Reach end point.
			Hit(hp);
			getPlayScene()->Hit();
			reachEndTime = 0;
			return;
		}
		Engine::Point target = path.back() * PlayScene::BlockSize + Engine::Point(PlayScene::BlockSize / 2, PlayScene::BlockSize / 2);
		Engine::Point vec = target - Position;
		// Add up the distances:
		// 1. to path.back()
		// 2. path.back() to border
		// 3. All intermediate block size
		// 4. to end point
		reachEndTime = (vec.Magnitude() + (path.size() - 1) * PlayScene::BlockSize - remainSpeed) / speed;
		Engine::Point normalized = vec.Normalize();
		if (remainSpeed - vec.Magnitude() > 0) {
			Position = target;
			path.pop_back();
			remainSpeed -= vec.Magnitude();
		}
		else {
			Velocity = normalized * remainSpeed / deltaTime;
			remainSpeed = 0;
		}
	}
	Rotation = atan2(Velocity.y, Velocity.x);
	Sprite::Update(deltaTime);

    Enemy* enemy;
	if(!triggered) {
        triggered = true;
        time_stamp = al_get_time();
    }
    time = al_get_time();
    if(time - time_stamp > 5 && triggered) {
        getPlayScene()->EnemyGroup->AddNewObject(enemy = new BlueNormalEnemy(Position.x, Position.y));
        enemy->UpdatePath(getPlayScene()->mapDistance);
        //enemy->Update(0);
        time_stamp = time;
    }
}

