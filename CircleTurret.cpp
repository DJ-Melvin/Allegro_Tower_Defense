#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "AudioHelper.hpp"
#include "CircleBullet.hpp"
#include "Group.hpp"
#include "CircleTurret.hpp"
#include "RocketBullet.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"
#include "ShootEffect.hpp"
const int CircleTurret::Price = 50;
CircleTurret::CircleTurret(float x, float y) :
    Turret("play/tower-base.png", "play/CircleTurret.png", x, y, 150, Price, 0.5) {
    // Move center downward, since we the turret head is slightly biased upward
    id = 4;
    Anchor.y += 8.0f / GetBitmapHeight();
}
void CircleTurret::CreateBullet() {
    //Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    //Engine::Point normalized = diff.Normalize();
    // Change bullet position to the front of the gun barrel.
    Engine::Point point_1 = Engine::Point(radius, 0);
    Engine::Point point_2 = Engine::Point(0, radius);
    Engine::Point point_3 = Engine::Point(-radius, 0);
    Engine::Point point_4 = Engine::Point(0, -radius);

    Child[0] = new CircleBullet(Position + point_1, 0, this, this);
    getPlayScene()->BulletGroup->AddNewObject(Child[0]);

    Child[1] = new CircleBullet(Position + point_2, ALLEGRO_PI / 2, this, this);
    getPlayScene()->BulletGroup->AddNewObject(Child[1]);

    Child[2] = new CircleBullet(Position + point_3, ALLEGRO_PI, this, this);
    getPlayScene()->BulletGroup->AddNewObject(Child[2]);

    Child[3] = new CircleBullet(Position + point_4, 3 * ALLEGRO_PI / 2, this, this);
    getPlayScene()->BulletGroup->AddNewObject(Child[3]);

    AudioHelper::PlayAudio("gun.wav");
}
void CircleTurret::Update(float deltaTime) {
    imgBase.Position = Position;
	imgBase.Tint = Tint;
    if (!Enabled)
		return;
    if(!bullet_num) {
        CreateBullet();
        bullet_num = 4;
    }
}
CircleTurret::~CircleTurret() {
    for(int i = 0; i < 4; i++) {
        if(Child[i] != nullptr) {
            getPlayScene()->BulletGroup->RemoveObject(Child[i]->GetObjectIterator());
        }
    }
}
