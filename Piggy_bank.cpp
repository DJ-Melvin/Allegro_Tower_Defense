#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "AudioHelper.hpp"
#include "Group.hpp"
#include "Piggy_bank.hpp"
#include "PlayScene.hpp"
#include "Point.hpp"

const int PiggyTurret::Price = 60;
PiggyTurret::PiggyTurret(float x, float y) :
    // TODO 3 (1/5): You can imitate the 2 files: 'PlugGunTurret.hpp', 'PlugGunTurret.cpp' to create a new turret.
    Turret("play/tower-base.png", "play/piggy_bank.png", x, y, 200, Price, 1.5) {
    id = 6;
    // Move center downward, since we the turret head is slightly biased upward
    //Anchor.y += 8.0f / GetBitmapHeight();
}
void PiggyTurret::CreateBullet() {
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
void PiggyTurret::Update(float deltaTime) {
    Sprite::Update(deltaTime);
    imgBase.Position = Position;
	imgBase.Tint = Tint;
    if(!Enabled) return;
    if(!triggered) {
        triggered = true;
        time_stamp = al_get_time();
    }
    time = al_get_time();
    if(time - time_stamp > 3 && triggered) {
        getPlayScene()->EarnMoney(10);
        time_stamp = time;
    }
}



