#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "AudioHelper.hpp"
#include "RocketBullet.hpp"
#include "Group.hpp"
#include "UpgradeTurret.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"
#include "ShootEffect.hpp"
const int UpgradeTurret::Price = 30;
UpgradeTurret::UpgradeTurret(float x, float y) :
    Turret("play/tower-base.png", "play/turret-2.png", x, y, 350, Price, 0.5) {
    id = 3;
    // Move center downward, since we the turret head is slightly biased upward
    Anchor.y += 8.0f / GetBitmapHeight();
}
void UpgradeTurret::CreateBullet() {
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2 ), sin(Rotation - ALLEGRO_PI / 2 ));
    Engine::Point diff_2 = Engine::Point(-sin(Rotation - ALLEGRO_PI / 2 ), cos(Rotation - ALLEGRO_PI / 2 ));

    float rotation = atan2(diff.y, diff.x);
    Engine::Point normalized = diff.Normalize();
    Engine::Point normalized_2 = diff_2.Normalize();
    // Change bullet position to the front of the gun barrel.
    getPlayScene()->BulletGroup->AddNewObject(new RocketBullet(Position + normalized * 36, diff, rotation, this));
    getPlayScene()->BulletGroup->AddNewObject(new RocketBullet(Position + normalized * 36 - diff_2 * 18, diff, rotation, this));

    getPlayScene()->EffectGroup->AddNewObject(new ShootEffect(Position.x + normalized.x * 36, Position.y + normalized.y * 36));
    //getPlayScene()->EffectGroup->AddNewObject(new ShootEffect(Position.x + normalized.x * 36, Position.y + normalized.y * 36));
    AudioHelper::PlayAudio("gun.wav");
}


