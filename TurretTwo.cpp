#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "AudioHelper.hpp"
#include "RocketBullet.hpp"
#include "Group.hpp"
#include "TurretTwo.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"
#include "ShootEffect.hpp"
const int TurretTwo::Price = 30;
TurretTwo::TurretTwo(float x, float y) :
    Turret("play/tower-base.png", "play/turret-1.png", x, y, 350, Price, 0.5) {
    // Move center downward, since we the turret head is slightly biased upward
    id = 2;
    Anchor.y += 8.0f / GetBitmapHeight();
}
void TurretTwo::CreateBullet() {
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    float rotation = atan2(diff.y, diff.x);
    Engine::Point normalized = diff.Normalize();
    // Change bullet position to the front of the gun barrel.
    getPlayScene()->BulletGroup->AddNewObject(new RocketBullet(Position + normalized * 36, diff, rotation, this));
    getPlayScene()->EffectGroup->AddNewObject(new ShootEffect(Position.x + normalized.x * 36, Position.y + normalized.y * 36));
    AudioHelper::PlayAudio("gun.wav");
}

