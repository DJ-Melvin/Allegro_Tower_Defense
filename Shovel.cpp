#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "AudioHelper.hpp"
#include "Group.hpp"
#include "ShootEffect.hpp"
#include "PlayScene.hpp"
#include "Turret.hpp"
#include "Shovel.hpp"
#include "Point.hpp"

const int Shovel::Price = 0;
Shovel::Shovel(float x, float y) :
    // TODO 3 (1/5): You can imitate the 2 files: 'PlugGunTurret.hpp', 'PlugGunTurret.cpp' to create a new turret.
    Turret("play/tower-base.png", "play/shovel.png", x, y, 0, Price, 1.5) {
    id = 5;
    // Move center downward, since we the turret head is slightly biased upward
    Anchor.y += 8.0f / GetBitmapHeight();
}
void Shovel::CreateBullet() {
    /*
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    float rotation = atan2(diff.y, diff.x);
    Engine::Point normalized = diff.Normalize();
    // Change bullet position to the front of the gun barrel.
    getPlayScene()->BulletGroup->AddNewObject(new WoodBullet(Position + normalized * 36, diff, rotation, this));
    // TODO 4 (2/2): Add a ShootEffect here. Remember you need to include the class.
    getPlayScene()->EffectGroup->AddNewObject(new ShootEffect(Position.x + normalized.x * 36, Position.y + normalized.y * 36));
    AudioHelper::PlayAudio("gun.wav");
    */
}
