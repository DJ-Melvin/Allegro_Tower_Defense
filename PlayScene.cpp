#include <allegro5/allegro.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <vector>
#include <queue>
#include <string>
#include <memory>

#include "AudioHelper.hpp"
#include "DirtyEffect.hpp"
#include "Enemy.hpp"
#include "GameEngine.hpp"
#include "Group.hpp"
#include "IObject.hpp"
#include "Image.hpp"
#include "Label.hpp"
// Turret
#include "PlugGunTurret.hpp"
#include "TurretTwo.hpp"
#include "UpgradeTurret.hpp"
#include "CircleTurret.hpp"
#include "Plane.hpp"
#include "Shovel.hpp"
#include "Shifter.hpp"
#include "Piggy_bank.hpp"
// Enemy
#include "Boss.hpp"
#include "RedNormalEnemy.hpp"
#include "BlueNormalEnemy.hpp"
#include "StrongEnemy.hpp"
#include "PlayScene.hpp"
#include "Resources.hpp"
#include "Sprite.hpp"
#include "Turret.hpp"
#include "TurretButton.hpp"
#include "LOG.hpp"

bool PlayScene::DebugMode = false;
const std::vector<Engine::Point> PlayScene::directions = { Engine::Point(-1, 0), Engine::Point(0, -1), Engine::Point(1, 0), Engine::Point(0, 1) };
const int PlayScene::MapWidth = 20, PlayScene::MapHeight = 13;
const int PlayScene::BlockSize = 64;
const float PlayScene::DangerTime = 7.61;
const Engine::Point PlayScene::SpawnGridPoint = Engine::Point(-1, 0);
const Engine::Point PlayScene::EndGridPoint = Engine::Point(MapWidth, MapHeight - 1);
// TODO 5 (2/3): Set the cheat code correctly.
const std::vector<int> PlayScene::code = { ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN, ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_ENTER };
Engine::Point PlayScene::GetClientSize() {
	return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
}
void PlayScene::Initialize() {
	// TODO 6 (1/2): There's a bug in this file, which crashes the game when you win. Try to find it.
	// TODO 6 (2/2): There's a bug in this file, which doesn't update the player's life correctly when getting the first attack. Try to find it.
	mapState.clear();
	keyStrokes.clear();
	ticks = 0;
	deathCountDown = -1;
	lives = 10;
	money = 150;
	SpeedMult = 1;
	// Add groups from bottom to top.
	AddNewObject(TileMapGroup = new Group());
	AddNewObject(GroundEffectGroup = new Group());
	AddNewObject(DebugIndicatorGroup = new Group());
	AddNewObject(TowerGroup = new Group());
	AddNewObject(EnemyGroup = new Group());
	AddNewObject(BulletGroup = new Group());
	AddNewObject(EffectGroup = new Group());
	// Should support buttons.
	AddNewControlObject(UIGroup = new Group());
	ReadMap();
	ReadEnemyWave();
	mapDistance = CalculateBFSDistance();
	ConstructUI();
	imgTarget = new Engine::Image("play/target.png", 0, 0);
	imgTarget->Visible = false;
	preview = nullptr;
	UIGroup->AddNewObject(imgTarget);
	// Preload Lose Scene
	deathBGMInstance = Engine::Resources::GetInstance().GetSampleInstance("astronomia.ogg");
	Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
	// Start BGM.
	bgmId = AudioHelper::PlayBGM("play.ogg");
	if (!mute)
        bgmInstance = AudioHelper::PlaySample("play.ogg", true, AudioHelper::BGMVolume);
    else
        bgmInstance = AudioHelper::PlaySample("play.ogg", true, 0.0);
}
void PlayScene::Terminate() {
	AudioHelper::StopBGM(bgmId);
	AudioHelper::StopSample(deathBGMInstance);
	deathBGMInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
	IScene::Terminate();
}
void PlayScene::Update(float deltaTime) {
	// If we use deltaTime directly, then we might have Bullet-through-paper problem.
	// Reference: Bullet-Through-Paper
	if (SpeedMult == 0)
		deathCountDown = -1;
	else if (deathCountDown != -1)
		SpeedMult = 1;
	// Calculate danger zone.
	std::vector<float> reachEndTimes;
	for (auto& it : EnemyGroup->GetObjects()) {
		reachEndTimes.push_back(dynamic_cast<Enemy*>(it)->reachEndTime);
	}
	// Can use Heap / Priority-Queue instead. But since we won't have too many enemies, sorting is fast enough.
	std::sort(reachEndTimes.begin(), reachEndTimes.end());
	float newDeathCountDown = -1;
	int danger = lives;
	for (auto& it : reachEndTimes) {
		if (it <= DangerTime) {
			danger--;
			if (danger <= 0) {
				// Death Countdown
				float pos = DangerTime - it;
				if (it > deathCountDown) {
					// Restart Death Count Down BGM.
					AudioHelper::StopSample(deathBGMInstance);
					if (SpeedMult != 0)
						deathBGMInstance = AudioHelper::PlaySample("astronomia.ogg", false, AudioHelper::BGMVolume, pos);
				}
				float alpha = pos / DangerTime;
				alpha = std::max(0, std::min(255, static_cast<int>(alpha * alpha * 255)));
				dangerIndicator->Tint = al_map_rgba(255, 255, 255, alpha);
				newDeathCountDown = it;
				break;
			}
		}
	}
	deathCountDown = newDeathCountDown;
	if (SpeedMult == 0)
		AudioHelper::StopSample(deathBGMInstance);
	if (deathCountDown == -1 && lives > 0) {
		AudioHelper::StopSample(deathBGMInstance);
		dangerIndicator->Tint.a = 0;
	}
	if (SpeedMult == 0)
		deathCountDown = -1;
	for (int i = 0; i < SpeedMult; i++) {
		IScene::Update(deltaTime);
		// Check if we should create new enemy.
		ticks += deltaTime;
		if (enemyWaveData.empty()) {
			if (EnemyGroup->GetObjects().empty()) {
				// Free resources.
				//delete TileMapGroup;
				//delete GroundEffectGroup;
				//delete DebugIndicatorGroup;
				//delete TowerGroup;
				//delete EnemyGroup;
				//delete BulletGroup;
				//delete EffectGroup;
				//delete UIGroup;
				//delete imgTarget;

                Engine::GameEngine::GetInstance().ChangeScene("win");
                std::cout << "win--------------------------------" << std::endl;
			}
			continue;
		}
		auto current = enemyWaveData.front();
		if (ticks < current.second)
			continue;
		ticks -= current.second;
		enemyWaveData.pop_front();
		const Engine::Point SpawnCoordinate = Engine::Point(SpawnGridPoint.x * BlockSize + BlockSize / 2, SpawnGridPoint.y * BlockSize + BlockSize / 2);
		Enemy* enemy;
		switch (current.first) {
		case 1:
			EnemyGroup->AddNewObject(enemy = new RedNormalEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
			break;
        case 2:
            EnemyGroup->AddNewObject(enemy = new BlueNormalEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
			break;
        case 3:
            EnemyGroup->AddNewObject(enemy = new StrongEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
			break;
        case 4:
            EnemyGroup->AddNewObject(enemy = new Boss(SpawnCoordinate.x, SpawnCoordinate.y));
			break;

		// TODO 2 (2/3): You need to modify 'resources/enemy1.txt', or 'resources/enemy2.txt' to spawn the new enemy.
		// The format is "[EnemyId] [TimeDelay] [Repeat]".
		// TODO 2 (3/3): Enable the creation of the new enemy.
		default:
			continue;
		}
		enemy->UpdatePath(mapDistance);
		// Compensate the time lost.
		enemy->Update(ticks);
	}
	if (preview) {
		preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
		// To keep responding when paused.
		preview->Update(deltaTime);
	}
}
void PlayScene::Draw() const {
	IScene::Draw();
	if (DebugMode) {
		// Draw reverse BFS distance on all reachable blocks.
		for (int i = 0; i < MapHeight; i++) {
			for (int j = 0; j < MapWidth; j++) {
				if (mapDistance[i][j] != -1) {
					// Not elegant nor efficient, but it's quite enough for debugging.
					Engine::Label label(std::to_string(mapDistance[i][j]), "pirulen.ttf", 32, (j + 0.5) * BlockSize, (i + 0.5) * BlockSize);
					label.Anchor = Engine::Point(0.5, 0.5);
					label.Draw();
				}
			}
		}
	}
}
void PlayScene::OnMouseDown(int button, int mx, int my) {
	if ((button & 1) && !imgTarget->Visible && preview) {
		// Cancel turret construct.
		UIGroup->RemoveObject(preview->GetObjectIterator());
		preview = nullptr;
	}
	IScene::OnMouseDown(button, mx, my);
}
void PlayScene::OnMouseMove(int mx, int my) {
	IScene::OnMouseMove(mx, my);
	const int x = mx / BlockSize;
	const int y = my / BlockSize;
	if (!preview || x < 0 || x >= MapWidth || y < 0 || y >= MapHeight) {
		imgTarget->Visible = false;
		return;
	}
	imgTarget->Visible = true;
	imgTarget->Position.x = x * BlockSize;
	imgTarget->Position.y = y * BlockSize;
}
void PlayScene::OnMouseUp(int button, int mx, int my) {
	IScene::OnMouseUp(button, mx, my);
	if (!imgTarget->Visible)
		return;
	const int x = mx / BlockSize;
	const int y = my / BlockSize;
	if (button & 1) {
        if (!preview) return;
        if(preview->GetID() == 7) { // shifter
            can_cost = false;
            if(mapState[y][x] == TILE_DIRT || mapState[y][x] == TILE_FLOOR) return;
            if (!CheckSpaceValid(x, y)) return;

            // Remove preview
            preview->GetObjectIterator()->first = false;
            UIGroup->RemoveObject(preview->GetObjectIterator());

            auto position = preview->Position;
            Turret* Target;
            int tmp_id;

            for (auto& it : TowerGroup->GetObjects()) {
                    Target = dynamic_cast<Turret*>(it);
                if (Target->Position.x == x * BlockSize + BlockSize / 2 && Target->Position.y == y * BlockSize + BlockSize / 2) {
                    tmp_id = Target->GetID();
                    //TowerGroup->RemoveObject(Target->GetObjectIterator());
                    //EarnMoney(Target->GetPrice());
                    TowerGroup->RemoveObject(Target->GetObjectIterator());
                    std::cout << "-----------move" << std::endl;
                    break;
                }
            }
            preview = nullptr;
            if(tmp_id == 1) {
                std::cout << tmp_id << std::endl;
                //UIBtnClicked(0);
                preview = new PlugGunTurret(0, 0);
            }
            else if(tmp_id == 2) {
                std::cout << tmp_id << std::endl;
                preview = new TurretTwo(0, 0);
            }
            else if(tmp_id == 3) {
                std::cout << tmp_id << std::endl;
                preview = new UpgradeTurret(0, 0);
            }
            else if(tmp_id == 4) {
                std::cout << tmp_id << std::endl;
                preview = new CircleTurret(0, 0);
            }
            else if(tmp_id == 6) {
                std::cout << tmp_id << std::endl;
                preview = new PiggyTurret(0, 0);
            }
            preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
            preview->Tint = al_map_rgba(255, 255, 255, 200);
            preview->Enabled = false;
            preview->Preview = true;
            UIGroup->AddNewObject(preview);
            mapState[y][x] = TILE_FLOOR;
            OnMouseMove(mx, my);
        }
        else if(preview->GetID() == 5) { // shovel
            //can_cost = true;
            //std::cout << "test" << std::endl;
            if(mapState[y][x] == TILE_DIRT || mapState[y][x] == TILE_FLOOR) {
                //std::cout << "test-----------------" << std::endl;
                return;
            }
             if (!CheckSpaceValid(x, y)) {
                //std::cout << "test" << std::endl;
                return;
            }

            // Remove Preview.
            //preview->Enabled = true;
            //preview->Preview = false;
            preview->GetObjectIterator()->first = false;
            UIGroup->RemoveObject(preview->GetObjectIterator());

            Turret* Target;
            for (auto& it : TowerGroup->GetObjects()) {
                    Target = dynamic_cast<Turret*>(it);
                if (Target->Position.x == x * BlockSize + BlockSize / 2 && Target->Position.y == y * BlockSize + BlockSize / 2) {
                    if(can_cost) EarnMoney(Target->GetPrice() / 2);

                    TowerGroup->RemoveObject(Target->GetObjectIterator());

                     std::cout << "remove-----------" << std::endl;
                     //mapState[x * BlockSize + BlockSize / 2][y * BlockSize + BlockSize / 2] = TILE_FLOOR;
                    //Target->lockedTurrets.push_back(this);
                    //lockedTurretIterator = std::prev(Target->lockedTurrets.end());
                    break;
                }
            }
            mapState[y][x] = TILE_FLOOR;
            //preview->Update(0);
            preview = nullptr;
            OnMouseMove(mx, my);
            //std::cout<<"line266" << std::endl;
        }
		else if (mapState[y][x] == TILE_DIRT || mapState[y][x] == TILE_FLOOR) {
            //can_cost = true;
			// Check if valid.
			if (!CheckSpaceValid(x, y)) {
				Engine::Sprite* sprite;
				GroundEffectGroup->AddNewObject(sprite = new DirtyEffect("play/target-invalid.png", 1, x * BlockSize + BlockSize / 2, y * BlockSize + BlockSize / 2));
				sprite->Rotation = 0;
				return;
			}
			// Purchase.
            if(can_cost) EarnMoney(-preview->GetPrice());
            // Remove Preview.
            preview->GetObjectIterator()->first = false;
            UIGroup->RemoveObject(preview->GetObjectIterator());
            // Construct real turret.
            preview->Position.x = x * BlockSize + BlockSize / 2;
            preview->Position.y = y * BlockSize + BlockSize / 2;
            preview->Enabled = true;
            preview->Preview = false;
            preview->Tint = al_map_rgba(255, 255, 255, 255);
            TowerGroup->AddNewObject(preview);
            // To keep responding when paused.

            if(preview->GetID() == 1) mapState[y][x] = TILE_PLUG;
            else if(preview->GetID() == 2) mapState[y][x] = TILE_GREEN;
            else if(preview->GetID() == 4) mapState[y][x] = TILE_CIRCLE;
            else if(preview->GetID() == 6) mapState[y][x] = TILE_PIGGY;
            //preview->Update(0);
            preview = nullptr;
            OnMouseMove(mx, my);
		}
		else if(preview->GetID() == 2){
            //can_cost = true;
            if(mapState[y][x] == TILE_PLUG || mapState[y][x] == TILE_RED || mapState[y][x] == TILE_CIRCLE || mapState[y][x] == TILE_PIGGY) return;
            //if(mapState[y][x] == TILE_GREEN && preview->GetID() != 2) return;
            // Purchase.
            if(mapState[y][x] == TILE_GREEN) {
                // Remove Preview.
                preview->GetObjectIterator()->first = false;
                UIGroup->RemoveObject(preview->GetObjectIterator());
                //TowerGroup->RemoveObject(preview->GetObjectIterator());
                for(auto& it : TowerGroup->GetObjects()) {
                        Turret* turret = dynamic_cast<Turret*>(it);
                    if(turret->Position.x == x * BlockSize + BlockSize / 2 && turret->Position.y == y * BlockSize + BlockSize / 2) {
                        TowerGroup->RemoveObject(turret->GetObjectIterator());
                        break;
                    }
                }
                //TowerGroup->DeleteObject(x, y);
                // Construct real turret.
                UIGroup->RemoveObject(preview->GetObjectIterator());
                preview = new UpgradeTurret(0, 0);
                preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
                preview->Tint = al_map_rgba(255, 255, 255, 200);
                preview->Enabled = false;
                preview->Preview = true;
                UIGroup->AddNewObject(preview);
                if(can_cost) EarnMoney(-preview->GetPrice());

                preview->GetObjectIterator()->first = false;
                UIGroup->RemoveObject(preview->GetObjectIterator());
                preview->Position.x = x * BlockSize + BlockSize / 2;
                preview->Position.y = y * BlockSize + BlockSize / 2;
                preview->Enabled = true;
                preview->Preview = false;
                preview->Tint = al_map_rgba(255, 255, 255, 255);
                TowerGroup->AddNewObject(preview);
                // To keep responding when paused.
                //preview->Update(0);
                // Remove Preview.
                preview = nullptr;
                mapState[y][x] = TILE_RED;
                OnMouseMove(mx, my);
            }
		}
	}
}
void PlayScene::OnKeyDown(int keyCode) {
	IScene::OnKeyDown(keyCode);
	if (keyCode == ALLEGRO_KEY_TAB) {
		// TODO 5 (1/3): Set Tab as a code to active / de-active the debug mode.
		DebugMode = !DebugMode;
	}
	else {
		keyStrokes.push_back(keyCode);
		if (keyStrokes.size() > code.size())
			keyStrokes.pop_front();
        std::vector<int>::const_iterator it;
        std::list<int>::const_iterator it_list;
        int code_size = 1;
        for(it = code.begin(), it_list = keyStrokes.begin(); it != code.end() && it_list != keyStrokes.end(); it++, it_list++, code_size++) {
            if(*it_list != *it) break;
            else if(code_size == code.size()) {
                EffectGroup->AddNewObject(new Plane());
                money += 10000;
                lives += 10;
                UIMoney->Text = std::string("$") + std::to_string(this->money);
                UILives->Text = std::string("Life ") + std::to_string(this->lives);
            }
        }
		// TODO 5 (3/3): Check whether the input sequence corresponds to the code. If so, active a plane and earn 10000 money.
        // Active a plane : EffectGroup->AddNewObject(new Plane());
		// Earn money : money += 10000;
	}
	if (keyCode == ALLEGRO_KEY_Q) {
		// Hotkey for PlugGunTurret.
		UIBtnClicked(0);
	}
	// TODO 3 (5/5): Make the W key to create the new turret.
	else if (keyCode == ALLEGRO_KEY_W) {
		// Hotkey for new turret.
		UIBtnClicked(1);
	}
	else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
		// Hotkey for Speed up.
		SpeedMult = keyCode - ALLEGRO_KEY_0;
	}
	else if (keyCode == ALLEGRO_KEY_M) {
		// Hotkey for mute / unmute.
        if (mute)
            AudioHelper::ChangeSampleVolume(bgmInstance, AudioHelper::BGMVolume);
        else
            AudioHelper::ChangeSampleVolume(bgmInstance, 0.0);
        mute = !mute;
	}
}
void PlayScene::Hit() {
	UILives->Text = std::string("Life ") + std::to_string(--lives);
	if (lives <= 0) {
		Engine::GameEngine::GetInstance().ChangeScene("lose");
	}
}
int PlayScene::GetMoney() const {
	return money;
}
void PlayScene::EarnMoney(int money) {
	this->money += money;
	UIMoney->Text = std::string("$") + std::to_string(this->money);
}
void PlayScene::ReadMap() {
	std::string filename = std::string("resources/map") + std::to_string(MapId) + ".txt";
	// Read map file.
	char c;
	std::vector<bool> mapData;
	std::ifstream fin(filename);
	while (fin >> c) {
		switch (c) {
		case '0': mapData.push_back(false); break;
		case '1': mapData.push_back(true); break;
		case '\n':
		case '\r':
			if (static_cast<int>(mapData.size()) / MapWidth != 0)
				throw std::ios_base::failure("Map data is corrupted.");
			break;
		default: throw std::ios_base::failure("Map data is corrupted.");
		}
	}
	fin.close();
	// Validate map data.
	if (static_cast<int>(mapData.size()) != MapWidth * MapHeight)
		throw std::ios_base::failure("Map data is corrupted.");
	// Store map in 2d array.
	mapState = std::vector<std::vector<TileType>>(MapHeight, std::vector<TileType>(MapWidth));
	for (int i = 0; i < MapHeight; i++) {
		for (int j = 0; j < MapWidth; j++) {
			const int num = mapData[i * MapWidth + j];
			mapState[i][j] = num ? TILE_FLOOR : TILE_DIRT;
			if (num)
				TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
			else
				TileMapGroup->AddNewObject(new Engine::Image("play/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
		}
	}
}
void PlayScene::ReadEnemyWave() {
	std::string filename = std::string("resources/enemy") + std::to_string(MapId) + ".txt";
	// Read enemy file.
	float type, wait, repeat;
	enemyWaveData.clear();
	std::ifstream fin(filename);
	while (fin >> type && fin >> wait && fin >> repeat) {
		for (int i = 0; i < repeat; i++)
			enemyWaveData.emplace_back(type, wait);
	}
	fin.close();
}
void PlayScene::ConstructUI() {
	// Background
	UIGroup->AddNewObject(new Engine::Image("play/sand.png", 1280, 0, 320, 832));
	// Text
	UIGroup->AddNewObject(new Engine::Label(std::string("Stage ") + std::to_string(MapId), "pirulen.ttf", 32, 1294, 0));
	UIGroup->AddNewObject(UIMoney = new Engine::Label(std::string("$") + std::to_string(money), "pirulen.ttf", 24, 1294, 48));
	UIGroup->AddNewObject(UILives = new Engine::Label(std::string("Life ") + std::to_string(lives), "pirulen.ttf", 24, 1294, 88));
	// Buttons
	ConstructButton(0, "play/turret-6.png", PlugGunTurret::Price);
	// TODO 3 (3/5): Create a button to support constructing the new turret.

    ConstructButton(1, "play/turret-1.png", TurretTwo::Price);

    ConstructButton(2, "play/CircleTurret.png", CircleTurret::Price);

    ConstructButton(3, "play/shovel.png", Shovel::Price);

    ConstructButton(4, "play/piggy_bank.png", PiggyTurret::Price);

    ConstructButton(5, "play/move.png", Shifter::Price);

	int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
	int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
	int shift = 135 + 25;
	dangerIndicator = new Engine::Sprite("play/benjamin.png", w - shift, h - shift);
	dangerIndicator->Tint.a = 0;
	UIGroup->AddNewObject(dangerIndicator);
}

void PlayScene::ConstructButton(int id, std::string sprite, int price) {
	TurretButton* btn;
	if(id == 4 || id == 5) {
        //std::cout << "-----------------------------" << std::endl;
        btn = new TurretButton("play/floor.png", "play/dirt.png",
            Engine::Sprite("play/tower-base.png", 1294 + (id - 4) * 76, 204, 0, 0, 0, 0),
            Engine::Sprite(sprite, 1294 + (id - 4) * 76, 204, 0, 0, 0, 0)
            , 1294 + (id - 4) * 76, 204, price);
	}
	else {
        btn = new TurretButton("play/floor.png", "play/dirt.png",
            Engine::Sprite("play/tower-base.png", 1294 + id * 76, 136, 0, 0, 0, 0),
            Engine::Sprite(sprite, 1294 + id * 76, 136 - 8, 0, 0, 0, 0)
            , 1294 + id * 76, 136, price);
	}
	// Reference: Class Member Function Pointer and std::bind.
	btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, id));
	UIGroup->AddNewControlObject(btn);
}

void PlayScene::UIBtnClicked(int id) {
	if (preview) {
		UIGroup->RemoveObject(preview->GetObjectIterator());
        preview = nullptr;
    }
	if (id == 0 && money >= PlugGunTurret::Price) {
        preview = new PlugGunTurret(0, 0);
        can_cost = true;
	}
    else if(id == 1 && money >= TurretTwo::Price) {
        preview = new TurretTwo(0, 0);
        can_cost = true;
    }
    else if(id == 2 && money >= CircleTurret::Price) {
        preview = new CircleTurret(0, 0);
        can_cost = true;
    }
    else if(id == 3) {
        preview = new Shovel(0, 0);
        can_cost = true;
    }
    else if(id == 4 && money >= PiggyTurret::Price) {
        preview = new PiggyTurret(0, 0);
        can_cost = true;
    }
    else if(id == 5) {
        preview = new Shifter(0, 0);
        can_cost = false;
    }
    else if(id == 99 && money >= UpgradeTurret::Price) {
        preview = new UpgradeTurret(0, 0);
        can_cost = true;
    }
	// TODO 3 (4/5): On the new turret button callback, create the new turret.
	if (!preview) return;
	preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
	preview->Tint = al_map_rgba(255, 255, 255, 200);
	preview->Enabled = false;
	preview->Preview = true;
	UIGroup->AddNewObject(preview);
	OnMouseMove(Engine::GameEngine::GetInstance().GetMousePosition().x, Engine::GameEngine::GetInstance().GetMousePosition().y);
}

bool PlayScene::CheckSpaceValid(int x, int y) {
	if (x < 0 || x >= MapWidth || y < 0 || y >= MapHeight)
		return false;
	auto map00 = mapState[y][x];
	mapState[y][x] = TILE_OCCUPIED;
	std::vector<std::vector<int>> map = CalculateBFSDistance();
	mapState[y][x] = map00;
	if (map[0][0] == -1)
		return false;
	for (auto& it : EnemyGroup->GetObjects()) {
		Engine::Point pnt;
		pnt.x = floor(it->Position.x / BlockSize);
		pnt.y = floor(it->Position.y / BlockSize);
		if (pnt.x < 0) pnt.x = 0;
		if (pnt.x >= MapWidth) pnt.x = MapWidth - 1;
		if (pnt.y < 0) pnt.y = 0;
		if (pnt.y >= MapHeight) pnt.y = MapHeight - 1;
		if (map[pnt.y][pnt.x] == -1)
			return false;
	}
	// All enemy have path to exit.
	mapState[y][x] = TILE_OCCUPIED;
	mapDistance = map;
	for (auto& it : EnemyGroup->GetObjects())
		dynamic_cast<Enemy*>(it)->UpdatePath(mapDistance);
	return true;
}
std::vector<std::vector<int>> PlayScene::CalculateBFSDistance() {
	// Reverse BFS to find path.
	std::vector<std::vector<int>> map(MapHeight, std::vector<int>(std::vector<int>(MapWidth, -1)));
	std::queue<Engine::Point> que;
	// Push end point.
	// BFS from end point.
	if (mapState[MapHeight - 1][MapWidth - 1] != TILE_DIRT)
		return map;
	que.push(Engine::Point(MapWidth - 1, MapHeight - 1));
	map[MapHeight - 1][MapWidth - 1] = 0;
	while (!que.empty()) {
		Engine::Point p = que.front();
		que.pop();
        for (auto &c : directions) {
            int x = p.x + c.x;
            int y = p.y + c.y;
            if (x < 0 || x >= MapWidth || y < 0 || y >= MapHeight ||
                map[y][x] != -1 || mapState[y][x] != TILE_DIRT) {
                continue;
            } else {
                map[y][x] = map[p.y][p.x] + 1;
                que.push(Engine::Point(x, y));
            }
        }
	}
	return map;
}