#include <allegro5/base.h>
#include <random>
#include <string>

#include "DirtyEffect.hpp"
#include "Enemy.hpp"
#include "CircleBullet.hpp"
#include "Group.hpp"
#include "PlayScene.hpp"
#include "Collider.hpp"
#include "Point.hpp"
#include "Bullet.hpp"
#include "Turret.hpp"
#include "CircleTurret.hpp"

class Turret;
CircleBullet::CircleBullet(Engine::Point position, float angle, Turret* parent, CircleTurret* real_parent) :
    Bullet("play/bullet-7.png", 400, 3, position, position, 0, parent) ,angle(angle){
        this->real_parent = real_parent;

}
void CircleBullet::OnExplode(Enemy* enemy) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(2, 5);
    getPlayScene()->GroundEffectGroup->AddNewObject(new DirtyEffect("play/dirty-3.png", dist(rng), enemy->Position.x, enemy->Position.y));
}
void CircleBullet::Update(float deltaTime) {
    angle += omega * deltaTime;
    if(angle > 2 * ALLEGRO_PI) angle -= 2 * ALLEGRO_PI;
	Position.x = parent->Position.x + radius * cos(angle);
	Position.y = parent->Position.y + radius * sin(angle);
	//Sprite::Update(deltaTime);
	PlayScene* scene = getPlayScene();
	// Can be improved by Spatial Hash, Quad Tree, ...
	// However simply loop through all enemies is enough for this program.
	for (auto& it : scene->EnemyGroup->GetObjects()) {
		Enemy* enemy = dynamic_cast<Enemy*>(it);
		if (!enemy->Visible)
			continue;
		if (Engine::Collider::IsCircleOverlap(Position, CollisionRadius, enemy->Position, enemy->CollisionRadius)) {
			OnExplode(enemy);
            enemy->Slow(0.5, 3);
			enemy->Hit(damage);
			getPlayScene()->BulletGroup->RemoveObject(objectIterator);
            for(int i = 0; i < 4; i++) {
                if(real_parent->Child[i] == this) {
                        real_parent->Child[i] = nullptr;
                        break;
                }
            }
			real_parent->bullet_num--;
			return;
		}
	}
	// Check if out of boundary.
}


