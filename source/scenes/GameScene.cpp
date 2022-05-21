#include "GameScene.h"

#include <box2d/b2_collision.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_world.h>
#include <cugl/cugl.h>

#include "../controllers/CollisionFiltering.h"
#include "../controllers/actions/Attack.h"
#include "../controllers/actions/Corrupt.h"
#include "../controllers/actions/Dash.h"
#include "../controllers/actions/Movement.h"
#include "../controllers/actions/OpenMap.h"
#include "../controllers/actions/Settings.h"
#include "../controllers/actions/TargetPlayer.h"
#include "../loaders/CustomScene2Loader.h"
#include "../models/RoomModel.h"
#include "../models/tiles/TileHelper.h"
#include "../models/tiles/Wall.h"
#include "../network/NetworkController.h"
#include "../network/structs/EnemyStructs.h"
#include "../network/structs/PlayerStructs.h"

#define SCENE_HEIGHT 720
#define CAMERA_SMOOTH_SPEED_FACTOR 300.0f
#define CAMERA_LARGEST_DIFF 200.0f
#define MIN_PLAYERS 4
#define MIN_BETRAYERS 1
#define ENERGY_BAR_UPDATE_SIZE 0.02f
/** Set cloud wrap x position based on width and scale of cloud layer **/
#define CLOUD_WRAP -960

bool GameScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets,
    const std::shared_ptr<level_gen::LevelGenerator>& level_gen,
    const std::shared_ptr<cugl::scene2::SceneNode>& map, bool is_betrayer,
    std::string display_name, std::unordered_map<int, int> color_ids) {
  if (_active) return false;
  _active = true;
  _state = RUN;

  _display_name = display_name;
  _has_sent_player_basic_info = false;
  _dead_enemy_cache.clear();

  // Initialize the scene to a locked width.

  cugl::Size dim = cugl::Application::get()->getDisplaySize();
  dim *= SCENE_HEIGHT / ((dim.width > dim.height) ? dim.width : dim.height);

  if (assets == nullptr || !cugl::Scene2::init(dim)) return false;

  _assets = assets;

  _world_node = cugl::scene2::OrderedNode::allocWithOrder(
      cugl::scene2::OrderedNode::Order::ASCEND);
  _world_node->setContentSize(dim);

  std::shared_ptr<cugl::Texture> target_texture =
      _assets->get<cugl::Texture>("target-player");
  auto target_icon_node = cugl::scene2::SpriteNode::alloc(target_texture, 1, 1);
  target_icon_node->setName("target-icon");
  target_icon_node->setVisible(false);
  _world_node->addChild(target_icon_node);

  _debug_node = cugl::scene2::SceneNode::alloc();
  _debug_node->setContentSize(dim);

  _map = map;
  _map->setContentSize(dim);
  _map->setPosition(dim / 2);
  _map->setScale(1.2f);
  _map->setVisible(false);

  _level_controller = LevelController::alloc(_assets, _world_node, _debug_node,
                                             level_gen, _map, is_betrayer);

  // Get the world from level controller and attach the listeners.
  _world = _level_controller->getWorld();
  _world->setGravity(cugl::Vec2::ZERO);
  _world->activateCollisionCallbacks(true);
  _world->onBeginContact = [this](b2Contact* contact) {
    this->beginContact(contact);
  };

  _grunt_controller =
      GruntController::alloc(_assets, _world, _world_node, _debug_node);
  _shotgunner_controller =
      ShotgunnerController::alloc(_assets, _world, _world_node, _debug_node);
  _tank_controller =
      TankController::alloc(_assets, _world, _world_node, _debug_node);
  _turtle_controller =
      TurtleController::alloc(_assets, _world, _world_node, _debug_node);

  setBetrayer(is_betrayer);

  _terminal_controller = TerminalController::alloc(_assets);

  _player_controller = PlayerController::alloc(_assets, _world, _world_node,
                                               _debug_node, color_ids);
  populate(dim);

  _world_node->doLayout();

  auto terminal_deposit_layer =
      assets->get<cugl::scene2::SceneNode>("terminal-deposit-scene");
  terminal_deposit_layer->setContentSize(dim);
  terminal_deposit_layer->doLayout();

  auto background_layer = assets->get<cugl::scene2::SceneNode>("background");
  background_layer->setContentSize(dim);
  background_layer->doLayout();

  _cloud_layer = assets->get<cugl::scene2::SceneNode>("clouds");
  _cloud_layer->setContentSize(dim);
  _cloud_layer->doLayout();

  _settings_scene = SettingsScene::alloc(_assets);
  _settings_scene->setPlayerController(_player_controller);
  _settings_scene->getNode()->setContentSize(dim);
  _settings_scene->getNode()->doLayout();

  // assign role screen depending on player role
  if (is_betrayer) {
    _role_layer = std::dynamic_pointer_cast<cugl::scene2::Button>(
        _assets->get<cugl::scene2::SceneNode>("betrayer-scene"));
  } else {
    _role_layer = std::dynamic_pointer_cast<cugl::scene2::Button>(
        _assets->get<cugl::scene2::SceneNode>("runner-scene"));
  }
  _role_layer->setContentSize(dim);
  _role_layer->doLayout();

  // wait for button click, then hide role screen
  _role_layer->addListener([this](const std::string& name, bool down) {
    if (down && _role_layer->isVisible()) {
      _role_layer->setVisible(false);
      InputController::get()->resume();
    }
  });
  _role_layer->activate();

  auto ui_layer = assets->get<cugl::scene2::SceneNode>("ui-scene");
  ui_layer->setContentSize(dim);
  ui_layer->doLayout();

  auto player_head = std::dynamic_pointer_cast<cugl::scene2::TexturedNode>(
      assets->get<cugl::scene2::SceneNode>("ui-scene_player-head"));
  int player_id = *(NetworkController::get()->getConnection()->getPlayerID());
  player_head->setTexture(assets->get<cugl::Texture>(
      "player-head-" + std::to_string(color_ids[player_id])));

  auto health_layer = assets->get<cugl::scene2::SceneNode>("health");
  health_layer->setContentSize(dim);
  health_layer->doLayout();

  auto energy_layer = assets->get<cugl::scene2::SceneNode>("energy");
  energy_layer->setContentSize(dim);
  energy_layer->doLayout();

  _health_bar = std::dynamic_pointer_cast<cugl::scene2::ProgressBar>(
      assets->get<cugl::scene2::SceneNode>("health_bar"));
  _energy_bar = std::dynamic_pointer_cast<cugl::scene2::ProgressBar>(
      assets->get<cugl::scene2::SceneNode>("energy_bar"));

  assets->get<cugl::scene2::SceneNode>("ui-scene_block-player")
      ->setVisible(!is_betrayer);
  assets->get<cugl::scene2::SceneNode>("ui-scene_infect-player")
      ->setVisible(is_betrayer);

  _energy_bar->setForegroundColor(cugl::Color4("#9ec1de"));
  if (is_betrayer) _energy_bar->setForegroundColor(cugl::Color4("#df7126"));

  auto win_layer = assets->get<cugl::scene2::SceneNode>("win-scene");
  win_layer->setContentSize(dim);
  win_layer->doLayout();
  win_layer->setVisible(false);

  _num_terminals_activated = 0;
  _num_terminals_corrupted = 0;
  auto corrupted_text =
      ui_layer->getChildByName<cugl::scene2::Label>("corrupted_num");
  std::string corrupted_msg =
      cugl::strtool::format(std::to_string(_num_terminals_corrupted));
  corrupted_text->setText(corrupted_msg);
  corrupted_text->setForeground(cugl::Color4::BLACK);

  auto name_text = ui_layer->getChildByName<cugl::scene2::Label>("name");
  name_text->setText(_display_name);
  name_text->setForeground(cugl::Color4::BLACK);

  auto role_text = ui_layer->getChildByName<cugl::scene2::Label>("role");
  std::string role_msg = "";
  if (_is_betrayer) {
    role_msg = "(BETRAYER)";
    role_text->setForeground(cugl::Color4::BLACK);
    role_text->setDropShadow(.75, -.75);
  } else {
    role_msg = "(RUNNER)";
    role_text->setForeground(cugl::Color4::BLACK);
  }
  role_text->setText(role_msg);

  _particle_world = cugl::scene2::SceneNode::alloc();
  _particle_screen = cugl::scene2::SceneNode::alloc();
  _particle_world->setContentSize(dim);
  _particle_screen->setContentSize(dim);
  _particle_screen->setName("particle_screen");
  _particle_controller =
      ParticleController::alloc(_particle_world, _particle_screen);
  _player_controller->setParticleController(_particle_controller);
  _level_controller->setParticleController(_particle_controller);

  _sound_controller = SoundController::alloc(_assets);
  _player_controller->setSoundController(_sound_controller);
  _grunt_controller->setSoundController(_sound_controller);
  _shotgunner_controller->setSoundController(_sound_controller);
  _tank_controller->setSoundController(_sound_controller);
  _turtle_controller->setSoundController(_sound_controller);

  cugl::Scene2::addChild(background_layer);
  cugl::Scene2::addChild(_cloud_layer);
  cugl::Scene2::addChild(_world_node);
  _world_node->addChild(_particle_world);
  cugl::Scene2::addChild(_map);
  cugl::Scene2::addChild(health_layer);
  cugl::Scene2::addChild(energy_layer);
  cugl::Scene2::addChild(ui_layer);
  cugl::Scene2::addChild(terminal_deposit_layer);
  cugl::Scene2::addChild(_role_layer);
  cugl::Scene2::addChild(_particle_screen);
  cugl::Scene2::addChild(_debug_node);
  cugl::Scene2::addChild(_settings_scene->getNode());
  _debug_node->setVisible(false);

  InputController::get()->init(_assets, cugl::Scene2::getBounds(), is_betrayer);

  InputController::get()->pause();

  _energy_particle = ParticleProps(ParticleProps::Type::PATH);
  _energy_particle.setLifeTime(0.7f)
      ->setScreenCoord(true)
      ->setColorStart(cugl::Color4(148, 183, 212, 200))
      ->setColorEnd(cugl::Color4(148, 183, 212, 120))
      ->setAngularSpeed(0.5f)
      ->setEasingFunctionPosX(cugl::EasingFunction::sineIn)
      ->setEasingFunctionPosY(cugl::EasingFunction::sineOut)
      ->setPathVariation(50.f)
      ->setSizeStart(5.0f)
      ->setSizeEnd(5.0f)
      ->setPositionVariation(2.0f, 2.0f);

  _deposit_particle_regular = ParticleProps(ParticleProps::Type::PATH);
  _deposit_particle_regular.setLifeTime(0.7f)
      ->setColorStart(cugl::Color4(148, 183, 212, 200))
      ->setColorEnd(cugl::Color4(148, 183, 212, 120))
      ->setAngularSpeed(0.5f)
      ->setEasingFunctionPosX(cugl::EasingFunction::sineOut)
      ->setEasingFunctionPosY(cugl::EasingFunction::sineIn)
      ->setPathVariation(50.f)
      ->setSizeStart(5.0f)
      ->setSizeEnd(5.0f)
      ->setPositionVariation(2.0f, 2.0f);
  if (is_betrayer) {
    _energy_particle.setColorStart(cugl::Color4(223, 113, 38, 200))
        ->setColorEnd(cugl::Color4(223, 113, 38, 120));
  }

  _deposit_particle_corrupted = ParticleProps(ParticleProps::Type::PATH);
  _deposit_particle_corrupted.setLifeTime(0.7f)
      ->setColorStart(cugl::Color4(223, 113, 38, 200))
      ->setColorEnd(cugl::Color4(223, 113, 38, 120))
      ->setAngularSpeed(0.5f)
      ->setEasingFunctionPosX(cugl::EasingFunction::sineOut)
      ->setEasingFunctionPosY(cugl::EasingFunction::sineIn)
      ->setPathVariation(50.f)
      ->setSizeStart(5.0f)
      ->setSizeEnd(5.0f)
      ->setPositionVariation(2.0f, 2.0f);

  return true;
}

void GameScene::dispose() {
  if (!_active) return;
  _state = NONE;

  InputController::get()->dispose();
  _active = false;

  _has_sent_player_basic_info = false;
  _sound_controller->stop();
  _dead_enemy_cache.clear();
  _world_node->removeAllChildren();
  _debug_node->removeAllChildren();
  _role_layer->setVisible(true);
  _role_layer->deactivate();
  _role_layer->setDown(false);
  _settings_scene->dispose();
  removeAllChildren();

  _world_node = nullptr;
  _debug_node = nullptr;
  _role_layer = nullptr;
  _cloud_layer = nullptr;

  _world = nullptr;

  _settings_scene = nullptr;
  _terminal_controller = nullptr;
  _sound_controller = nullptr;
  _player_controller = nullptr;
  _grunt_controller = nullptr;
  _shotgunner_controller = nullptr;
  _tank_controller = nullptr;
  _turtle_controller = nullptr;
  _level_controller = nullptr;
}

void GameScene::populate(cugl::Size dim) {
  if (auto id = NetworkController::get()->getConnection()->getPlayerID()) {
    auto player = _player_controller->makePlayer(*id);
    _player_controller->setMyPlayer(player);
    player->setDisplayName(_display_name);
    player->setBetrayer(_is_betrayer);
  }

  _terminal_controller->setPlayerController(_player_controller);
  _terminal_controller->setLevelController(_level_controller);
  _level_controller->setPlayerController(_player_controller);

  // Add physics enabled tiles to world node, debug node and box2d physics
  // world.
  auto loader = std::dynamic_pointer_cast<cugl::CustomScene2Loader>(
      _assets->access<cugl::scene2::SceneNode>());

  for (std::shared_ptr<Wall> wall : TileHelper::getTile<Wall>(_world_node)) {
    _world->addObstacle(wall->initBox2d());
    wall->getObstacle()->setDebugColor(cugl::Color4::GREEN);
    wall->getObstacle()->setDebugScene(_debug_node);
  }

  _num_terminals = 0;
  for (std::shared_ptr<Terminal> terminal :
       TileHelper::getTile<Terminal>(_world_node)) {
    _num_terminals += 1;
  }

  // Debug code.
  _player_controller->getMyPlayer()->setDebugScene(_debug_node);
  _player_controller->getMyPlayer()->setDebugColor(cugl::Color4f::BLACK);

  std::shared_ptr<RoomModel> current_room =
      _level_controller->getLevelModel()->getCurrentRoom();
  _player_controller->getMyPlayer()->setRoomId(current_room->getKey());
  for (std::shared_ptr<EnemyModel> enemy : current_room->getEnemies()) {
    enemy->setDebugColor(cugl::Color4f::BLACK);
    enemy->setDebugScene(_debug_node);
  }
}

bool GameScene::checkCooperatorWin() {
  return _num_terminals / 2 < _num_terminals_activated;
}

bool GameScene::checkBetrayerWin() {
  return _num_terminals / 2 < _num_terminals_corrupted;
}

void GameScene::update(float timestep) {
  if (NetworkController::get()->isConnectionSet()) {
    sendNetworkInfo();
    // Receives information and calls listeners (eg. processData).
    NetworkController::get()->update();
  }

  _health_bar->setProgress(_player_controller->getMyPlayer()->getHealth() /
                           50.0f);

  // Animate energy update.
  float target_energy_amt =
      _player_controller->getMyPlayer()->getEnergy() / 100.0f;
  if (_energy_bar->getProgress() < target_energy_amt) {
    float diff = _energy_bar->getProgress() + ENERGY_BAR_UPDATE_SIZE;
    _energy_bar->setProgress(std::min(diff, target_energy_amt));
  } else if (_energy_bar->getProgress() > target_energy_amt) {
    float diff = _energy_bar->getProgress() - ENERGY_BAR_UPDATE_SIZE;
    _energy_bar->setProgress(std::max(diff, target_energy_amt));
  }

  if (_player_controller->getMyPlayer()->getRespawning()) {
    _player_controller->getMyPlayer()->setRespawning(false);
    // TODO: Change this to new terminal system. #233
    _level_controller->moveToCenterOfRoom(
        _level_controller->getLevelModel()->getSpawnRoom()->getKey());
  }

  if (checkCooperatorWin() || checkBetrayerWin()) _state = DONE;

  cugl::Application::get()->setClearColor(cugl::Color4f::BLACK);

  InputController::get()->update();

  _player_controller->update(timestep);
  _terminal_controller->update(timestep);
  _level_controller->update(timestep);
  _sound_controller->update(timestep);
  _particle_controller->update(timestep);

  _num_terminals_activated = _terminal_controller->getNumTerminalsActivated();
  _num_terminals_corrupted = _terminal_controller->getNumTerminalsCorrupted();

  if (InputController::get<OpenMap>()->didOpenMap()) {
    _map->setVisible(!_map->isVisible());
  }

  if (InputController::get<Settings>()->didOpenSettings()) {
    _settings_scene->setActive(true);
    InputController::get()->pause();
  }
  _settings_scene->update();

  switch (_settings_scene->getChoice()) {
    case SettingsScene::Choice::LEAVE:
      _state = LEAVE;
      _settings_scene->setActive(false);
      if (NetworkController::get()->isHost()) {
        NetworkController::get()->send(NC_HOST_END_GAME);
      } else {
        auto info = cugl::PlayerIdInfo::alloc();
        info->player_id = _player_controller->getMyPlayer()->getPlayerId();
        NetworkController::get()->sendOnlyToHost(NC_CLIENT_END_GAME, info);
      }
      break;
    case SettingsScene::Choice::RESUME:
      _settings_scene->setActive(false);
      InputController::get()->resume();
      break;
    default:
      break;
  }

  // Cooperator block ability.
  if (!_player_controller->getMyPlayer()->isBetrayer()) {
    auto target_player = InputController::get<TargetPlayer>();
    if (target_player->didChangeTarget()) {
      int current_room_id = _player_controller->getMyPlayer()->getRoomId();
      bool found_player = false;
      bool others_in_room = false;
      int first_found_player = -1;
      for (auto it : _player_controller->getPlayers()) {
        std::shared_ptr<Player> player = it.second;
        if (player->getRoomId() == current_room_id &&
            player->getPlayerId() !=
                _player_controller->getMyPlayer()->getPlayerId()) {
          others_in_room = true;
          if (first_found_player == -1) {
            first_found_player = player->getPlayerId();
          }
          if (target_player->getTarget() == -1 ||
              !target_player->hasSeenPlayerId(player->getPlayerId())) {
            // Sets the target to a fresh player
            target_player->setTarget(player->getPlayerId());
            auto target_icon_node =
                _world_node->getChildByName<cugl::scene2::SceneNode>(
                    "target-icon");
            target_icon_node->setPosition(
                player->getPlayerNode()->getPosition());
            target_icon_node->setVisible(true);
            found_player = true;
            break;
          }
        }
      }
      // Cycled through all players and they have all already been visited, go
      // back to the first
      if (!found_player && others_in_room && first_found_player != -1) {
        target_player->clearDirtyPlayers();
        target_player->setTarget(first_found_player);
      }
    }

    auto target_icon_node =
        _world_node->getChildByName<cugl::scene2::SceneNode>("target-icon");
    if (target_player->getTarget() != -1) {
      for (auto it : _player_controller->getPlayers()) {
        std::shared_ptr<Player> player = it.second;
        if (player->getRoomId() ==
                _player_controller->getMyPlayer()->getRoomId() &&
            player->getPlayerId() == target_player->getTarget()) {
          target_icon_node->setPosition(player->getPlayerNode()->getPosition());
          target_icon_node->setVisible(true);
        }
      }
    } else {
      target_icon_node->setVisible(false);
    }

    if (target_player->isActivatingTargetAction()) {
      sendBetrayalTargetInfo(_player_controller->getMyPlayer()->getPlayerId(),
                             target_player->getTarget());
      if (target_player->getTarget() != -1) {
        _player_controller->getPlayer(target_player->getTarget())
            ->getBlockIcon()
            ->setVisible(true);
      }
    }

    if (target_player->isCooldownFinished()) {
      sendBetrayalTargetInfo(_player_controller->getMyPlayer()->getPlayerId(),
                             target_player->getPrevTarget());
      if (target_player->getPrevTarget() != -1) {
        _player_controller->getPlayer(target_player->getPrevTarget())
            ->getBlockIcon()
            ->setVisible(false);
      }
    }
  }

  // Betrayer corrupt ability.
  if (_player_controller->getMyPlayer()->isBetrayer()) {
    if (!_player_controller->getMyPlayer()->getDead()) {
      auto blocked_ps = _player_controller->getMyPlayer()->getBlockedPlayers();
      if (InputController::get<Corrupt>()->pressCorrupt()) {
        for (auto p : _player_controller->getPlayerList()) {
          bool same_room =
              p->getRoomId() == _player_controller->getMyPlayer()->getRoomId();
          bool can_infect =
              blocked_ps.find(p->getPlayerId()) == blocked_ps.end();

          if (same_room && can_infect && !p->isBetrayer()) {
            sendBetrayalCorruptInfo(p->getPlayerId());
            _player_controller->getMyPlayer()->setCorrupted();
          } else if (!can_infect) {
            p->flashBlockIcon();
          }
        }
      }

      // Show all the block icons of the players that have blocked my player.
      for (auto it : _player_controller->getPlayers()) {
        if (it.first != _player_controller->getMyPlayer()->getPlayerId()) {
          bool has_blocked = blocked_ps.find(it.first) != blocked_ps.end();
          auto icon = it.second->getBlockIcon();
          if (icon) icon->setVisible(has_blocked);
        }
      }
    }
  }

  std::shared_ptr<RoomModel> current_room =
      _level_controller->getLevelModel()->getCurrentRoom();
  _player_controller->getMyPlayer()->setRoomId(current_room->getKey());

  std::unordered_set<int> enemy_update_rooms = getRoomIdsWithPlayers();
  for (auto room_id_to_update : enemy_update_rooms) {
    auto room_to_update =
        _level_controller->getLevelModel()->getRoom(room_id_to_update);
    updateEnemies(timestep, room_to_update);
  }

  // Also update the adjacent room enemies if is host.
  // Must check here or weird interactions occur with clients updating
  // adjacent rooms.
  if (_ishost) {
    std::unordered_set<int> adj_enemy_update_rooms =
        getAdjacentRoomIdsWithoutPlayers();
    for (auto room_id_to_update : adj_enemy_update_rooms) {
      auto room_to_update =
          _level_controller->getLevelModel()->getRoom(room_id_to_update);
      updateEnemies(timestep, room_to_update);
    }
  }

  // update cloud background layer
  _cloud_layer->setPositionX(_cloud_layer->getPositionX() + .3);
  if (_cloud_layer->getPositionX() >= 0) {
    _cloud_layer->setPositionX(CLOUD_WRAP);
  }

  updateCamera(timestep);

  _world->update(timestep);

  // ===== POST-UPDATE =======
  auto ui_layer = _assets->get<cugl::scene2::SceneNode>("ui-scene");

  auto name_text = ui_layer->getChildByName<cugl::scene2::Label>("name");
  name_text->setText(_display_name);
  name_text->setForeground(cugl::Color4::BLACK);

  auto activated_text =
      ui_layer->getChildByName<cugl::scene2::Label>("activated_num");
  std::string activated_msg =
      cugl::strtool::format(std::to_string(_num_terminals_activated));
  activated_text->setText(activated_msg);
  activated_text->setForeground(cugl::Color4::BLACK);

  auto corrupted_text =
      ui_layer->getChildByName<cugl::scene2::Label>("corrupted_num");
  std::string corrupted_msg =
      cugl::strtool::format(std::to_string(_num_terminals_corrupted));
  corrupted_text->setText(corrupted_msg);
  corrupted_text->setForeground(cugl::Color4::BLACK);

  auto player_count_text =
      ui_layer->getChildByName<cugl::scene2::Label>("players-in-room");
  int other_players_in_room_count = 0;
  for (auto it : _player_controller->getPlayers()) {
    std::shared_ptr<Player> player = it.second;
    if (player->getRoomId() == _player_controller->getMyPlayer()->getRoomId()) {
      other_players_in_room_count++;
    }
  }

  std::stringstream player_count_msg;
  player_count_msg << "x" << other_players_in_room_count;

  player_count_text->setText(player_count_msg.str());
  player_count_text->setForeground(cugl::Color4::BLACK);

  auto role_text = ui_layer->getChildByName<cugl::scene2::Label>("role");
  std::string role_msg = "";
  if (_is_betrayer) {
    role_msg = "(BETRAYER)";
    role_text->setForeground(cugl::Color4::BLACK);
    role_text->setDropShadow(.75, -.75);
  } else {
    role_msg = "(RUNNER)";
    role_text->setForeground(cugl::Color4::BLACK);
  }

  // POST-UPDATE
  // Check for disposal

  //  auto room_ids_with_players = getRoomIdsWithPlayers();
  auto room_id_player_count_map = getRoomPlayerCounts();
  for (auto it : room_id_player_count_map) {
    auto room_id = it.first;
    auto room = _level_controller->getLevelModel()->getRoom(room_id);
    std::vector<std::shared_ptr<EnemyModel>>& enemies = room->getEnemies();

    for (auto it = enemies.begin(); it != enemies.end(); it++) {
      auto enemy = *it;

      if (enemy->isReadyToDie()) {
        // Send particles if there's things to send.
        if (_player_controller->getMyPlayer()->getEnergy() < 100) {
          cugl::Vec2 end_pos = _energy_bar->getWorldPosition();
          end_pos.x += _energy_bar->getContentWidth() *
                       _energy_bar->getProgress() * 1.2f;
          _energy_particle.setPosStart(enemy->getNode()->getWorldPosition())
              ->setPosEnd(end_pos);
          _particle_controller->emit(_energy_particle, 5, 0.03f);
        }

        // Update player eneregies.
        if (NetworkController::get()->isHost()) {
          for (auto jt : _player_controller->getPlayers()) {
            std::shared_ptr<Player> player = jt.second;
            //            int player_count = room_id_player_count_map[room_id];

            if (player->getRoomId() == room_id) {
              // Give all players in the same room some energy if an enemy dies.
              player->setEnergy(player->getEnergy() + 5);
            }
          }
        }

        if (NetworkController::get()->isHost()) {
          _dead_enemy_cache.push_back(enemy->getEnemyId());
        }
        enemy->deleteAllProjectiles(_world, _world_node);
        enemy->deactivatePhysics(*_world->getWorld());
        room->getNode()->removeChild(enemy->getNode());
        _world->removeObstacle(enemy.get());
        enemies.erase(it--);
      } else {
        enemy->deleteProjectile(_world, _world_node);
      }
    }
  }

  _player_controller->getMyPlayer()->checkDeleteSlashes(_world, _world_node);
}

void GameScene::updateEnemies(float timestep, std::shared_ptr<RoomModel> room) {
  int room_id = room->getKey();
  // Update the enemy controllers
  for (std::shared_ptr<EnemyModel>& enemy : room->getEnemies()) {
    switch (enemy->getType()) {
      case EnemyModel::GRUNT: {
        _grunt_controller->update(_ishost, timestep, enemy,
                                  _player_controller->getPlayerList(), room_id);
        break;
      }
      case EnemyModel::SHOTGUNNER: {
        _shotgunner_controller->update(_ishost, timestep, enemy,
                                       _player_controller->getPlayerList(),
                                       room_id);
        break;
      }
      case EnemyModel::TANK: {
        _tank_controller->update(_ishost, timestep, enemy,
                                 _player_controller->getPlayerList(), room_id);
        break;
      }
      case EnemyModel::TURTLE: {
        _turtle_controller->update(_ishost, timestep, enemy,
                                   _player_controller->getPlayerList(),
                                   room_id);
        break;
      }
    }
  }
}

void GameScene::sendNetworkInfo() {
  if (auto player_id =
          NetworkController::get()->getConnection()->getPlayerID()) {
    _player_controller->getMyPlayer()->setPlayerId(*player_id);
  }

  if (NetworkController::get()->isHost()) {
    sendNetworkInfoHost();
  } else {
    sendNetworkInfoClient();
  }
}

/**
 * Broadcasts the relevant network information if a host.
 */
void GameScene::sendNetworkInfoHost() {
  if (!NetworkController::get()->isHost()) return;

  {
    std::vector<std::shared_ptr<cugl::Serializable>> all_player_info;
    for (auto it : _player_controller->getPlayers()) {
      std::shared_ptr<Player> player = it.second;

      auto info = cugl::PlayerInfo::alloc();

      info->player_id = player->getPlayerId();
      info->room_id = player->getRoomId();
      info->pos = player->getPosition();

      all_player_info.push_back(info);
    }

    NetworkController::get()->send(NC_HOST_ALL_PLAYER_INFO, all_player_info);
  }

  {
    std::vector<std::shared_ptr<cugl::Serializable>> player_basic_info;

    bool all_players_present =
        _player_controller->getPlayers().size() ==
        NetworkController::get()->getConnection()->getNumPlayers();

    bool all_player_info = all_players_present;
    for (auto it : _player_controller->getPlayers()) {
      all_player_info &= it.second->hasBasicInfoSentToHost();
    }

    if (all_player_info && !_has_sent_player_basic_info) {
      _has_sent_player_basic_info = true;

      for (auto it : _player_controller->getPlayers()) {
        std::shared_ptr<Player> player = it.second;

        auto info = cugl::BasicPlayerInfo::alloc();
        info->player_id = player->getPlayerId();
        info->name = player->getDisplayName();
        info->betrayer = player->isBetrayer();

        player_basic_info.push_back(info);
      }

      NetworkController::get()->send(NC_HOST_ALL_PLAYER_BASIC_INFO,
                                     player_basic_info);
    }
  }

  {
    //    cugl::Timestamp time;
    //    Uint64 millis =
    //    time.ellapsedMillis(_time_of_last_player_other_info_update);
    //
    //    if (millis > 5000) {
    //      _time_of_last_player_other_info_update.mark();
    std::vector<std::shared_ptr<cugl::Serializable>> all_player_info;
    for (auto it : _player_controller->getPlayers()) {
      std::shared_ptr<Player> player = it.second;

      auto info = cugl::PlayerOtherInfo::alloc();

      info->player_id = player->getPlayerId();
      info->energy = player->getEnergy();
      info->corruption = player->getCorruptedEnergy();

      all_player_info.push_back(info);
    }

    NetworkController::get()->send(NC_HOST_ALL_PLAYER_OTHER_INFO,
                                   all_player_info);
    //    }
  }

  auto room_ids_with_players = getRoomIdsWithPlayers();
  for (auto room_id : room_ids_with_players) {
    // get enemy info for the rooms that players are in
    auto room = _level_controller->getLevelModel()->getRoom(room_id);
    {
      std::vector<std::shared_ptr<cugl::Serializable>> enemy_info;
      for (std::shared_ptr<EnemyModel> enemy : room->getEnemies()) {
        auto info = cugl::EnemyInfo::alloc();

        info->enemy_id = enemy->getEnemyId();
        info->pos = enemy->getPosition();
        info->has_target = enemy->didAttack();
        info->target = enemy->getAttackDir();
        if (info->has_target) {
          // Make sure bullet & attack is only sent once
          enemy->clearAttackState();
        }
        // Serialize one enemy at a time to avoid reaching packet limit
        enemy_info.push_back(info);
      }
      if (enemy_info.size() > 0) {
        NetworkController::get()->send(NC_HOST_ALL_ENEMY_INFO, enemy_info);
      }
    }

    {
      cugl::Timestamp time;
      Uint64 millis =
          time.ellapsedMillis(_time_of_last_enemy_other_info_update);

      if (millis > 200) {
        _time_of_last_enemy_other_info_update.mark();

        std::vector<std::shared_ptr<cugl::Serializable>> enemy_info;

        for (std::shared_ptr<EnemyModel> enemy : room->getEnemies()) {
          auto info = cugl::EnemyOtherInfo::alloc();

          info->enemy_id = enemy->getEnemyId();
          info->health = enemy->getHealth();

          enemy_info.push_back(info);
        }

        // Go through all the enemies that have died between these other info
        // update calls (200ms), and then clear the cache.
        for (int enemy_id : _dead_enemy_cache) {
          auto info = cugl::EnemyOtherInfo::alloc();
          info->enemy_id = enemy_id;
          info->health = -1;

          enemy_info.push_back(info);
        }
        _dead_enemy_cache.clear();

        NetworkController::get()->send(NC_HOST_ALL_ENEMY_OTHER_INFO,
                                       enemy_info);
      }
    }
  }
}

/**
 * Broadcasts the relevant network information if a client.
 */
void GameScene::sendNetworkInfoClient() {
  if (NetworkController::get()->isHost()) return;

  {
    auto info = cugl::PlayerInfo::alloc();

    info->player_id = _player_controller->getMyPlayer()->getPlayerId();
    info->room_id = _player_controller->getMyPlayer()->getRoomId();
    info->pos = _player_controller->getMyPlayer()->getPosition();

    // Send individual player information.
    NetworkController::get()->sendOnlyToHost(NC_CLIENT_ONE_PLAYER_INFO, info);
  }

  // Send basic info only once.
  if (!_player_controller->getMyPlayer()->hasBasicInfoSentToHost()) {
    _player_controller->getMyPlayer()->setBasicInfoSentToHost(true);

    auto info = cugl::BasicPlayerInfo::alloc();

    info->player_id = _player_controller->getMyPlayer()->getPlayerId();
    info->name = _display_name;
    info->betrayer = _is_betrayer;

    // Send individual player information.
    NetworkController::get()->sendOnlyToHost(NC_CLIENT_PLAYER_BASIC_INFO, info);
  }
}

void GameScene::sendEnemyHitNetworkInfo(int player_id, int enemy_id,
                                        float amount) {
  auto info = cugl::EnemyHitInfo::alloc();
  info->enemy_id = enemy_id;
  info->player_id = player_id;
  info->amount = amount;

  NetworkController::get()->sendOnlyToHostOrProcess(NC_CLIENT_ENEMY_HIT_INFO,
                                                    info);
}

void GameScene::sendBetrayalTargetInfo(int runner_id, int target_player_id) {
  auto betrayal_info = cugl::JsonValue::allocObject();

  auto action_player_info = cugl::JsonValue::alloc((long)(runner_id));
  betrayal_info->appendChild(action_player_info);
  action_player_info->setKey("runner_id");

  auto target_player_info = cugl::JsonValue::alloc((long)(target_player_id));
  betrayal_info->appendChild(target_player_info);
  target_player_info->setKey("target_player_id");

  NetworkController::get()->sendOnlyToHostOrProcess(NC_BETRAYAL_TARGET_INFO,
                                                    betrayal_info);
}

/*
 * This simply passes on the disable message on from the host to clients for
 * now. In the future the host can do server-side logic
 */
void GameScene::sendDisablePlayerInfo(int runner_id, int target_player_id) {
  auto betrayal_info = cugl::JsonValue::allocObject();

  auto action_player_info = cugl::JsonValue::alloc((long)(runner_id));
  betrayal_info->appendChild(action_player_info);
  action_player_info->setKey("runner_id");

  auto target_player_info = cugl::JsonValue::alloc((long)(target_player_id));
  betrayal_info->appendChild(target_player_info);
  target_player_info->setKey("target_player_id");

  NetworkController::get()->sendAndProcess(NC_SEND_DISABLE_PLAYER_INFO,
                                           betrayal_info);
}

void GameScene::sendBetrayalCorruptInfo(int corrupt_player_id) {
  auto betrayal_info = cugl::JsonValue::allocObject();

  auto corrupt_player_info = cugl::JsonValue::alloc((long)(corrupt_player_id));
  betrayal_info->appendChild(corrupt_player_info);
  corrupt_player_info->setKey("corrupt_player_id");

  NetworkController::get()->sendOnlyToHostOrProcess(
      NC_SEND_BETRAYAL_CORRUPT_INFO, betrayal_info);
}

/**
 * Processes data sent over the network.
 *
 * Once connection is established, all data sent over the network consistes of
 * byte vectors. This function is a call back function to process that data.
 * Note that this function may be called *multiple times* per animation frame,
 * as the messages can come from several sources.
 *
 * This is where we handle the gameplay. All connected devices should
 * immediately change their color when directed by the following method.
 * Changing the color means changing the clear color of the entire {@link
 * Application}.
 *
 * @param data  The data received
 */
void GameScene::processData(
    const Sint32& code,
    const cugl::CustomNetworkDeserializer::CustomMessage& msg) {
  switch (code) {
    case NC_HOST_END_GAME: {
      _state = LEAVE;
    } break;

    case NC_CLIENT_END_GAME: {
      auto info = std::dynamic_pointer_cast<cugl::PlayerIdInfo>(
          std::get<std::shared_ptr<cugl::Serializable>>(msg));

      bool end_game = (_player_controller->getPlayers().size() <= MIN_PLAYERS);
      end_game |= _player_controller->getNumberBetrayers() <= MIN_BETRAYERS &&
                  _player_controller->getMyPlayer()->isBetrayer();

      if (end_game) {
        _state = LEAVE;
        NetworkController::get()->send(NC_HOST_END_GAME);
      } else {
        NetworkController::get()->sendAndProcess(NC_HOST_REMOVE_PLAYER, info);
      }
    } break;

    case NC_HOST_REMOVE_PLAYER: {
      auto info = std::dynamic_pointer_cast<cugl::PlayerIdInfo>(
          std::get<std::shared_ptr<cugl::Serializable>>(msg));
      _player_controller->removePlayer(info->player_id);
    } break;

    case NC_HOST_ALL_ENEMY_INFO: {
      auto all_enemy =
          std::get<std::vector<std::shared_ptr<cugl::Serializable>>>(msg);

      for (std::shared_ptr<cugl::Serializable>& info_ : all_enemy) {
        auto info = std::dynamic_pointer_cast<cugl::EnemyInfo>(info_);
        std::shared_ptr<EnemyModel> enemy =
            _level_controller->getEnemy(info->enemy_id);

        if (enemy != nullptr) {
          enemy->setPosition(info->pos);
          if (info->has_target) enemy->setAttack(true);
          enemy->setAttackDir(info->target);
        }
      }
    } break;

    case NC_HOST_ALL_ENEMY_OTHER_INFO: {
      auto all_enemy =
          std::get<std::vector<std::shared_ptr<cugl::Serializable>>>(msg);

      for (std::shared_ptr<cugl::Serializable>& info_ : all_enemy) {
        auto info = std::dynamic_pointer_cast<cugl::EnemyOtherInfo>(info_);

        std::shared_ptr<EnemyModel> enemy =
            _level_controller->getEnemy(info->enemy_id);
        if (enemy != nullptr) {
          enemy->setHealth(info->health);
        }
      }
    } break;

    case NC_CLIENT_ENEMY_HIT_INFO: {
      auto info = std::dynamic_pointer_cast<cugl::EnemyHitInfo>(
          std::get<std::shared_ptr<cugl::Serializable>>(msg));

      auto enemy = _level_controller->getEnemy(info->enemy_id);

      if (enemy != nullptr) {
        auto player = _player_controller->getPlayer(info->player_id);
        enemy->takeDamageWithKnockback(player->getPosition(), info->amount);
        player->setEnergy(player->getEnergy() + 0.8f);
      }
    } break;

    case NC_BETRAYAL_TARGET_INFO: {
      if (NetworkController::get()->isHost()) {
        auto target_data = std::get<std::shared_ptr<cugl::JsonValue>>(msg);
        int runner_id = target_data->getInt("runner_id");
        int player_id = target_data->getInt("target_player_id");
        sendDisablePlayerInfo(runner_id, player_id);
      }
    } break;

    case NC_SEND_DISABLE_PLAYER_INFO: {
      auto target_data = std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int runner_id = target_data->getInt("runner_id");
      int player_id = target_data->getInt("target_player_id");

      if (player_id == _player_controller->getMyPlayer()->getPlayerId()) {
        // Blocks the player from corrupting for 1 minute.
        _player_controller->getMyPlayer()->toggleBlockPlayerOnBetrayer(
            runner_id);
      }
    } break;

    case NC_SEND_BETRAYAL_CORRUPT_INFO: {
      if (NetworkController::get()->isHost()) {
        auto corrupt_data = std::get<std::shared_ptr<cugl::JsonValue>>(msg);
        int player_id = corrupt_data->getInt("corrupt_player_id");
        auto corrupt_player = _player_controller->getPlayer(player_id);

        corrupt_player->turnEnergyCorrupted(20);
      }
    }
  }
}

void GameScene::setConnection(
    const std::shared_ptr<cugl::NetworkConnection>& network) {
  NetworkController::get()->init(network);
  NetworkController::get()->addListener(
      [=](const Sint32& code,
          const cugl::CustomNetworkDeserializer::CustomMessage& msg) {
        this->processData(code, msg);
      });
  NetworkController::get()->setDisconnectListener([=]() { _state = LEAVE; });
}

void GameScene::beginContact(b2Contact* contact) {
  b2Fixture* fx1 = contact->GetFixtureA();
  b2Fixture* fx2 = contact->GetFixtureB();

  void* fx1_d = (void*)fx1->GetUserData().pointer;
  void* fx2_d = (void*)fx2->GetUserData().pointer;

  std::string fx1_name;
  if (static_cast<std::string*>(fx1_d) != nullptr) {
    fx1_name.assign(*static_cast<std::string*>(fx1_d));
  }
  std::string fx2_name;
  if (static_cast<std::string*>(fx2_d) != nullptr) {
    fx2_name.assign(*static_cast<std::string*>(fx2_d));
  }

  b2Body* body1 = fx1->GetBody();
  b2Body* body2 = fx2->GetBody();

  cugl::physics2::Obstacle* ob1 = static_cast<cugl::physics2::Obstacle*>(
      (void*)body1->GetUserData().pointer);
  cugl::physics2::Obstacle* ob2 = static_cast<cugl::physics2::Obstacle*>(
      (void*)body2->GetUserData().pointer);

  if (!ob1 || !ob2) return;

  if (fx1_name == "enemy_hitbox" &&
      ob2 == _player_controller->getSword().get()) {
    float damage = 20;
    EnemyModel::EnemyType type = dynamic_cast<EnemyModel*>(ob1)->getType();
    if (type == EnemyModel::EnemyType::TURTLE) damage = 3;
    // Show hit on client-side without potentially causing de-sync with host
    // (0 dmg)
    _level_controller->getEnemy(dynamic_cast<EnemyModel*>(ob1)->getEnemyId())
        ->takeDamage(0);

    sendEnemyHitNetworkInfo(_player_controller->getMyPlayer()->getPlayerId(),
                            dynamic_cast<EnemyModel*>(ob1)->getEnemyId(),
                            damage);
  } else if (fx2_name == "enemy_hitbox" &&
             ob1 == _player_controller->getSword().get()) {
    float damage = 20;
    EnemyModel::EnemyType type = dynamic_cast<EnemyModel*>(ob2)->getType();
    if (type == EnemyModel::EnemyType::TURTLE) damage = 3;
    // Show hit on client-side without potentially causing de-sync with host
    // (0 dmg)
    _level_controller->getEnemy(dynamic_cast<EnemyModel*>(ob2)->getEnemyId())
        ->takeDamage(0);

    sendEnemyHitNetworkInfo(_player_controller->getMyPlayer()->getPlayerId(),
                            dynamic_cast<EnemyModel*>(ob2)->getEnemyId(),
                            damage);
  }

  if (fx1_name == "enemy_hitbox" &&
      ob2 == _player_controller->getMyPlayer().get()) {
    Player::State player_state = _player_controller->getMyPlayer()->getState();
    if (player_state == Player::State::DASHING) {
      // Show hit on client-side without potentially causing de-sync with host
      // (0 dmg)
      _level_controller->getEnemy(dynamic_cast<EnemyModel*>(ob1)->getEnemyId())
          ->takeDamage(0);
      sendEnemyHitNetworkInfo(_player_controller->getMyPlayer()->getPlayerId(),
                              dynamic_cast<EnemyModel*>(ob1)->getEnemyId(),
                              5.0f);
    }
  } else if (fx2_name == "enemy_hitbox" &&
             ob1 == _player_controller->getMyPlayer().get()) {
    Player::State player_state = _player_controller->getMyPlayer()->getState();
    if (player_state == Player::State::DASHING) {
      // Show hit on client-side without potentially causing de-sync with host
      // (0 dmg)
      _level_controller->getEnemy(dynamic_cast<EnemyModel*>(ob2)->getEnemyId())
          ->takeDamage(0);
      sendEnemyHitNetworkInfo(_player_controller->getMyPlayer()->getPlayerId(),
                              dynamic_cast<EnemyModel*>(ob2)->getEnemyId(),
                              5.0f);
    }
  }

  if (fx1_name == "enemy_damage" &&
      ob2 == _player_controller->getMyPlayer().get() &&
      dynamic_cast<EnemyModel*>(ob1)->getAttackCooldown() < 18) {
    if (_player_controller->getMyPlayer().get() == ob2) {
      _player_controller->getMyPlayer()->takeDamage();
    }
  } else if (fx2_name == "enemy_damage" &&
             ob1 == _player_controller->getMyPlayer().get() &&
             dynamic_cast<EnemyModel*>(ob2)->getAttackCooldown() < 18) {
    if (_player_controller->getMyPlayer().get() == ob1) {
      _player_controller->getMyPlayer()->takeDamage();
    }
  }

  if (ob1->getName() == "projectile" &&
      fx2_name == "player_projectile_sensor") {
    if (_player_controller->getMyPlayer().get() == ob2) {
      _player_controller->getMyPlayer()->takeDamage();
    }
    dynamic_cast<Projectile*>(ob1)->setFrames(0);  // Destroy the projectile
  } else if (ob2->getName() == "projectile" &&
             fx1_name == "player_projectile_sensor") {
    if (_player_controller->getMyPlayer().get() == ob1) {
      _player_controller->getMyPlayer()->takeDamage();
    }
    dynamic_cast<Projectile*>(ob2)->setFrames(0);  // Destroy the projectile
  }

  if (fx1_name == "enemy_hitbox" && ob2->getName() == "slash") {
    // Show hit on client-side without potentially causing de-sync with host
    // (0 dmg)
    _level_controller->getEnemy(dynamic_cast<EnemyModel*>(ob1)->getEnemyId())
        ->takeDamage(0);
    sendEnemyHitNetworkInfo(_player_controller->getMyPlayer()->getPlayerId(),
                            dynamic_cast<EnemyModel*>(ob1)->getEnemyId(), 30);
  } else if (fx2_name == "enemy_hitbox" && ob1->getName() == "slash") {
    // Show hit on client-side without potentially causing de-sync with host
    // (0 dmg)
    _level_controller->getEnemy(dynamic_cast<EnemyModel*>(ob2)->getEnemyId())
        ->takeDamage(0);
    sendEnemyHitNetworkInfo(_player_controller->getMyPlayer()->getPlayerId(),
                            dynamic_cast<EnemyModel*>(ob2)->getEnemyId(), 30);
  }

  if (ob1->getName() == "projectile" &&
      ob2 == _player_controller->getSword().get()) {
    dynamic_cast<Projectile*>(ob1)->setFrames(0);  // Destroy the projectile
  } else if (ob2->getName() == "projectile" &&
             ob1 == _player_controller->getSword().get()) {
    dynamic_cast<Projectile*>(ob2)->setFrames(0);  // Destroy the projectile
  }

  if (fx1_name.find("door") != std::string::npos &&
      ob2 == _player_controller->getMyPlayer().get()) {
    _level_controller->changeRoom(fx1_name);
  } else if (fx2_name.find("door") != std::string::npos &&
             ob1 == _player_controller->getMyPlayer().get()) {
    _level_controller->changeRoom(fx2_name);
  }

  if (fx1_name == "terminal-sensor" && ob1 != nullptr &&
      ob2 == _player_controller->getMyPlayer().get()) {
    cugl::Vec2 start_pos = _energy_bar->getWorldPosition();
    start_pos.x +=
        _energy_bar->getContentWidth() * _energy_bar->getProgress() * 1.2f;
    start_pos = _world_node->worldToNodeCoords(start_pos);

    Terminal* terminal =
        static_cast<Terminal*>((void*)ob1->getUserDataPointer());
    if (terminal != nullptr) {
      cugl::Vec2 end_pos = ob1->getPosition() + terminal->getContentSize() / 2;

      _deposit_particle_regular.setPosStart(start_pos)->setPosEnd(end_pos);
      _deposit_particle_corrupted.setPosStart(start_pos)->setPosEnd(end_pos);

      std::shared_ptr<Player> player = _player_controller->getMyPlayer();

      if (!terminal->isFilled() && player != nullptr) {
        if (player->getEnergy() > 0) {
          int num = 10 * player->getEnergy() / 100.0f + 1;
          if (player->isBetrayer()) {
            _particle_controller->emit(_deposit_particle_corrupted, num, 0.02f);
          } else {
            _particle_controller->emit(_deposit_particle_regular, num, 0.02f);
          }
        }

        if (player->getCorruptedEnergy() > 0) {
          int num = 10 * player->getCorruptedEnergy() / 100.0f + 1;
          _particle_controller->emit(_deposit_particle_corrupted, num, 0.02f);
        }
      }
    }

    std::shared_ptr<RoomModel> room =
        _level_controller->getLevelModel()->getCurrentRoom();
    if (room != nullptr) _terminal_controller->depositEnergy(room->getKey());

  } else if (fx2_name == "terminal-sensor" && ob2 != nullptr &&
             ob1 == _player_controller->getMyPlayer().get()) {
    cugl::Vec2 start_pos = _energy_bar->getWorldPosition();
    start_pos.x +=
        _energy_bar->getContentWidth() * _energy_bar->getProgress() * 1.2f;
    start_pos = _world_node->worldToNodeCoords(start_pos);

    Terminal* terminal =
        static_cast<Terminal*>((void*)ob2->getUserDataPointer());
    if (terminal != nullptr) {
      cugl::Vec2 end_pos = ob2->getPosition() + terminal->getContentSize() / 2;

      _deposit_particle_regular.setPosStart(start_pos)->setPosEnd(end_pos);
      _deposit_particle_corrupted.setPosStart(start_pos)->setPosEnd(end_pos);

      std::shared_ptr<Player> player = _player_controller->getMyPlayer();

      if (!terminal->isFilled() && player != nullptr) {
        if (player->getEnergy() > 0) {
          int num = 10 * player->getEnergy() / 100.0f + 1;
          if (player->isBetrayer()) {
            _particle_controller->emit(_deposit_particle_corrupted, num, 0.02f);
          } else {
            _particle_controller->emit(_deposit_particle_regular, num, 0.02f);
          }
        }

        if (player->getCorruptedEnergy() > 0) {
          int num = 10 * player->getCorruptedEnergy() / 100.0f + 1;
          _particle_controller->emit(_deposit_particle_corrupted, num, 0.02f);
        }
      }
    }

    std::shared_ptr<RoomModel> room =
        _level_controller->getLevelModel()->getCurrentRoom();
    if (room != nullptr) _terminal_controller->depositEnergy(room->getKey());
  }
}

void GameScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch) {
  Scene2::render(batch);
}

void GameScene::updateCamera(float timestep) {
  if (_settings_scene->isActive()) return;

  cugl::Vec2 desired_position =
      _world_node->getSize() / 2.0f -
      _player_controller->getMyPlayer()->getPosition();

  cugl::Vec2 smoothed_position;

  float speed = _player_controller->getMyPlayer()->getLinearVelocity().length();
  speed *= timestep;
  speed /= CAMERA_SMOOTH_SPEED_FACTOR;

  if (std::abs((desired_position - _world_node->getPosition()).length()) >
      CAMERA_LARGEST_DIFF) {
    speed = timestep * 5.0f;
  }

  speed = std::max(speed, timestep * 2.0f);

  cugl::Vec2::lerp(_world_node->getPosition(), desired_position, speed,
                   &smoothed_position);

  _world_node->setPosition(smoothed_position);
  _debug_node->setPosition(smoothed_position);
}
