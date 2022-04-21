#include "GameScene.h"

#include <box2d/b2_collision.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_world.h>
#include <cugl/cugl.h>

#include "../controllers/VotingInfo.h"
#include "../controllers/actions/Attack.h"
#include "../controllers/actions/Dash.h"
#include "../controllers/actions/Movement.h"
#include "../controllers/actions/OpenMap.h"
#include "../controllers/actions/TargetPlayer.h"
#include "../loaders/CustomScene2Loader.h"
#include "../models/RoomModel.h"
#include "../models/tiles/Wall.h"

#define SCENE_HEIGHT 720
#define CAMERA_SMOOTH_SPEED_FACTOR 300.0f
#define CAMERA_LARGEST_DIFF 200.0f

bool GameScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets,
    const std::shared_ptr<level_gen::LevelGenerator>& level_gen,
    bool is_betrayer, std::string display_name) {
  if (_active) return false;
  _active = true;

  _display_name = display_name;

  // Initialize the scene to a locked width.

  cugl::Size dim = cugl::Application::get()->getDisplaySize();
  dim *= SCENE_HEIGHT / ((dim.width > dim.height) ? dim.width : dim.height);

  if (assets == nullptr || !cugl::Scene2::init(dim)) return false;

  _assets = assets;

  _world_node = _assets->get<cugl::scene2::SceneNode>("world-scene");
  _world_node->setContentSize(dim);

  std::shared_ptr<cugl::Texture> target_texture =
      _assets->get<cugl::Texture>("target-player");
  auto target_icon_node = cugl::scene2::SpriteNode::alloc(target_texture, 1, 1);
  target_icon_node->setName("target-icon");
  target_icon_node->setVisible(false);
  _world_node->addChild(target_icon_node);

  _debug_node = cugl::scene2::SceneNode::alloc();
  _debug_node->setContentSize(dim);

  _level_controller =
      LevelController::alloc(_assets, _world_node, _debug_node, level_gen);

  _map = level_gen->getMap();
  _map->setContentSize(dim);
  _map->setPosition(dim / 2);
  _map->doLayout();
  _map->setVisible(false);

  for (std::shared_ptr<level_gen::Room>& room : level_gen->getRooms()) {
    for (std::shared_ptr<level_gen::Edge> edge : room->_edges) {
      edge->_node->setVisible(false);
    }
    room->_node->setVisible(false);
  }

  level_gen->getSpawnRoom()->_node->setVisible(true);

  // Get the world from level controller and attach the listeners.
  _world = _level_controller->getWorld();
  _world->setGravity(cugl::Vec2::ZERO);
  _world->activateCollisionCallbacks(true);
  _world->onBeginContact = [this](b2Contact* contact) {
    this->beginContact(contact);
  };
  _world->beforeSolve = [this](b2Contact* contact,
                               const b2Manifold* oldManifold) {
    this->beforeSolve(contact, oldManifold);
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

  populate(dim);

  _world_node->doLayout();

  auto terminal_voting_layer =
      assets->get<cugl::scene2::SceneNode>("terminal-voting-scene");
  terminal_voting_layer->setContentSize(dim);
  terminal_voting_layer->doLayout();

  auto background_layer = assets->get<cugl::scene2::SceneNode>("background");
  background_layer->setContentSize(dim);
  background_layer->doLayout();

  auto ui_layer = assets->get<cugl::scene2::SceneNode>("ui-scene");
  ui_layer->setContentSize(dim);
  ui_layer->doLayout();
  auto health_layer = assets->get<cugl::scene2::SceneNode>("health");
  health_layer->setContentSize(dim);
  health_layer->doLayout();

  _health_bar = std::dynamic_pointer_cast<cugl::scene2::ProgressBar>(
      assets->get<cugl::scene2::SceneNode>("health_bar"));

  auto win_layer = assets->get<cugl::scene2::SceneNode>("win-scene");
  win_layer->setContentSize(dim);
  win_layer->doLayout();
  win_layer->setVisible(false);

  auto target_player_button =
      ui_layer->getChildByName<cugl::scene2::Button>("target-player");
  target_player_button->setVisible(is_betrayer);

  auto enrage_button = ui_layer->getChildByName<cugl::scene2::Button>("enrage");
  enrage_button->setVisible(is_betrayer);

  auto timer_text = ui_layer->getChildByName<cugl::scene2::Label>("timer");
  std::string timer_msg = getTimerString();
  timer_text->setText(timer_msg);
  timer_text->setForeground(cugl::Color4::BLACK);

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
    role_msg = "(B)";
    role_text->setForeground(cugl::Color4::BLACK);
  } else {
    role_msg = "(C)";
    role_text->setForeground(cugl::Color4::BLACK);
  }
  role_text->setText(role_msg);

  _controllers.push_back(_player_controller->getHook());
  _controllers.push_back(_terminal_controller->getHook());
  _controllers.push_back(_level_controller->getHook());

  cugl::Scene2::addChild(background_layer);
  cugl::Scene2::addChild(_world_node);
  cugl::Scene2::addChild(_map);
  cugl::Scene2::addChild(health_layer);
  cugl::Scene2::addChild(ui_layer);
  cugl::Scene2::addChild(terminal_voting_layer);
  cugl::Scene2::addChild(win_layer);
  cugl::Scene2::addChild(_debug_node);
  _debug_node->setVisible(false);

  InputController::get()->init(_assets, cugl::Scene2::getBounds());
  InputController::get<TargetPlayer>()->setActive(is_betrayer);

  setMillisRemaining(900000);
  // Start the timer.
  _last_timestamp.mark();

  return true;
}

void GameScene::dispose() {
  if (!_active) return;
  InputController::get()->dispose();
  _active = false;
  _health_bar->dispose();
}

void GameScene::populate(cugl::Size dim) {
  // Initialize the player with texture and size, then add to world.
  std::shared_ptr<cugl::Texture> player = _assets->get<cugl::Texture>("player");

  auto my_player = Player::alloc(cugl::Vec2::ZERO, "Johnathan", _display_name);
  my_player->setBetrayer(_is_betrayer);

  auto pixelmix_font = _assets->get<cugl::Font>("pixelmix_extra_extra_small");
  auto player_node = cugl::scene2::SpriteNode::alloc(player, 9, 10);
  my_player->setPlayerNode(player_node, pixelmix_font);
  if (auto id = NetworkController::get()->getConnection()->getPlayerID()) {
    my_player->setPlayerId(*id);
  }

  _world_node->addChild(player_node);
  _world->addObstacle(my_player);

  _player_controller = PlayerController::alloc(my_player, _assets, _world,
                                               _world_node, _debug_node);
  _player_controller->addPlayer(my_player);
  _terminal_controller->setPlayerController(_player_controller);
  _level_controller->setPlayerController(_player_controller);

  // Add physics enabled tiles to world node, debug node and box2d physics
  // world.
  std::shared_ptr<cugl::CustomScene2Loader> loader =
      std::dynamic_pointer_cast<cugl::CustomScene2Loader>(
          _assets->access<cugl::scene2::SceneNode>());

  for (std::shared_ptr<BasicTile> tile : loader->getTiles("wall")) {
    auto wall = std::dynamic_pointer_cast<Wall>(tile);
    _world->addObstacle(wall->initBox2d());
    wall->getObstacle()->setDebugColor(cugl::Color4::GREEN);
    wall->getObstacle()->setDebugScene(_debug_node);
  }

  _num_terminals = 0;
  for (std::shared_ptr<BasicTile> tile : loader->getTiles("terminal")) {
    auto terminal = std::dynamic_pointer_cast<Terminal>(tile);
    _world->addObstacle(terminal->initBox2d());
    terminal->getObstacle()->setDebugColor(cugl::Color4::BLACK);
    terminal->getObstacle()->setDebugScene(_debug_node);
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
  if (_network) {
    sendNetworkInfo();
    // Receives information and calls listeners (eg. processData).
    NetworkController::get()->update();
  }
  _health_bar->setProgress(
      static_cast<float>(_player_controller->getMyPlayer()->getHealth()) / 100);

  if (_player_controller->getMyPlayer()->getRespawning()) {
    _player_controller->getMyPlayer()->setRespawning(false);
    auto latest_terminal_room = _level_controller->getLevelModel()->getRoom(
        _terminal_controller->getLatestTerminalRoomId());
    _level_controller->moveToCenterOfRoom(latest_terminal_room->getKey());
  }

  if (checkCooperatorWin()) {
    auto win_layer = _assets->get<cugl::scene2::SceneNode>("win-scene");
    auto text = win_layer->getChildByName<cugl::scene2::Label>("cooperator");
    std::string msg = cugl::strtool::format("Cooperators Win!");
    text->setText(msg);
    text->setForeground(cugl::Color4::GREEN);
    win_layer->setVisible(true);
  } else if (checkBetrayerWin()) {
    auto win_layer = _assets->get<cugl::scene2::SceneNode>("win-scene");
    auto text = win_layer->getChildByName<cugl::scene2::Label>("betrayer");
    std::string msg = cugl::strtool::format("Betrayers Win!");
    text->setText(msg);
    text->setForeground(cugl::Color4::BLACK);
    win_layer->setVisible(true);
  }

  cugl::Application::get()->setClearColor(cugl::Color4f::BLACK);

  InputController::get()->update();

  for (std::shared_ptr<Controller> controller : _controllers) {
    controller->update(timestep);
  }

  {  // Update the number of terminals activated and corrupted.
    _num_terminals_activated = 0;
    _num_terminals_corrupted = 0;
    for (auto it : _terminal_controller->getVotingInfo()) {
      std::shared_ptr<VotingInfo>& info = it.second;
      if (info->terminal_done) {
        if (info->was_activated) {
          _num_terminals_activated++;
        } else {
          _num_terminals_corrupted++;
        }
      }
    }
  }

  if (InputController::get<OpenMap>()->didOpenMap()) {
    _map->setVisible(!_map->isVisible());
  }

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
          target_icon_node->setPosition(player->getPlayerNode()->getPosition());
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
    sendBetrayalTargetInfo(target_player->getTarget());
  }

  std::shared_ptr<RoomModel> current_room =
      _level_controller->getLevelModel()->getCurrentRoom();
  _player_controller->getMyPlayer()->setRoomId(current_room->getKey());

  std::unordered_set<int> room_ids_with_players = getRoomIdsWithPlayers();
  for (auto room_id_to_update : room_ids_with_players) {
    auto room_to_update =
        _level_controller->getLevelModel()->getRoom(room_id_to_update);
    updateEnemies(timestep, room_to_update);
  }

  updateCamera(timestep);
  updateMillisRemainingIfHost();

  _world->update(timestep);

  // ===== POST-UPDATE =======
  auto ui_layer = _assets->get<cugl::scene2::SceneNode>("ui-scene");

  auto timer_text = ui_layer->getChildByName<cugl::scene2::Label>("timer");
  std::string timer_msg = getTimerString();
  timer_text->setText(timer_msg);
  timer_text->setForeground(cugl::Color4::BLACK);

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

  auto role_text = ui_layer->getChildByName<cugl::scene2::Label>("role");
  std::string role_msg = "";
  if (_is_betrayer) {
    role_msg = "(B)";
    role_text->setForeground(cugl::Color4::BLACK);
  } else {
    role_msg = "(C)";
    role_text->setForeground(cugl::Color4::BLACK);
  }

  // POST-UPDATE
  // Check for disposal
  std::vector<std::shared_ptr<EnemyModel>>& enemies =
      current_room->getEnemies();
  auto it = enemies.begin();
  while (it != enemies.end()) {
    auto enemy = *it;
    if (enemy->getHealth() <= 0) {
      enemy->deleteAllProjectiles(_world, _world_node);
      enemy->deactivatePhysics(*_world->getWorld());
      current_room->getNode()->removeChild(enemy->getNode());
      _world->removeObstacle(enemy.get());
      enemy->dispose();
      it = enemies.erase(it);
    } else {
      enemy->deleteProjectile(_world, _world_node);
      ++it;
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
  if (auto player_id = _network->getPlayerID()) {
    _player_controller->getMyPlayer()->setPlayerId(*player_id);
  }
  if (_ishost) {
    {
      std::vector<std::shared_ptr<cugl::JsonValue>> player_positions;
      std::set<int> rooms_checked_for_enemies;

      for (auto it : _player_controller->getPlayers()) {
        std::shared_ptr<Player> player = it.second;
        // get player info

        std::shared_ptr<cugl::JsonValue> player_info =
            cugl::JsonValue::allocObject();

        std::shared_ptr<cugl::JsonValue> player_id =
            cugl::JsonValue::alloc(static_cast<long>(player->getPlayerId()));
        player_info->appendChild(player_id);
        player_id->setKey("player_id");

        // send host-stored player's set display name to all clients
        std::shared_ptr<cugl::JsonValue> player_display_name =
            cugl::JsonValue::alloc(
                static_cast<std::string>(player->getDisplayName()));
        player_info->appendChild(player_display_name);
        player_display_name->setKey("player_display_name");

        std::shared_ptr<cugl::JsonValue> pos = cugl::JsonValue::allocArray();
        std::shared_ptr<cugl::JsonValue> pos_x =
            cugl::JsonValue::alloc(player->getPosition().x);
        std::shared_ptr<cugl::JsonValue> pos_y =
            cugl::JsonValue::alloc(player->getPosition().y);
        pos->appendChild(pos_x);
        pos->appendChild(pos_y);
        player_info->appendChild(pos);
        pos->setKey("position");

        std::shared_ptr<cugl::JsonValue> room =
            cugl::JsonValue::alloc(static_cast<long>(player->getRoomId()));
        player_info->appendChild(room);
        room->setKey("room");

        player_positions.push_back(player_info);

        int room_id = player->getRoomId();
        std::shared_ptr<RoomModel> player_room =
            _level_controller->getLevelModel()->getRoom(room_id);

        // continue loop when player_room not valid at the beginning
        if (player_room == nullptr) {
          continue;
        }

        // if room has already been checked, continue without adding enemies
        if (rooms_checked_for_enemies.count(room_id) > 0) {
          continue;
        }
        rooms_checked_for_enemies.insert(room_id);

        auto room_ids_with_players = getRoomIdsWithPlayers();
        for (auto room_id : room_ids_with_players) {
          // get enemy info only for the rooms that players are in
          auto room = _level_controller->getLevelModel()->getRoom(room_id);
          for (std::shared_ptr<EnemyModel> enemy : room->getEnemies()) {
            std::shared_ptr<cugl::JsonValue> enemy_info =
                cugl::JsonValue::allocObject();

            std::shared_ptr<cugl::JsonValue> enemy_id =
                cugl::JsonValue::alloc(static_cast<long>(enemy->getEnemyId()));
            enemy_info->appendChild(enemy_id);
            enemy_id->setKey("enemy_id");

            std::shared_ptr<cugl::JsonValue> pos =
                cugl::JsonValue::allocArray();
            std::shared_ptr<cugl::JsonValue> pos_x =
                cugl::JsonValue::alloc(enemy->getPosition().x);
            std::shared_ptr<cugl::JsonValue> pos_y =
                cugl::JsonValue::alloc(enemy->getPosition().y);
            pos->appendChild(pos_x);
            pos->appendChild(pos_y);
            enemy_info->appendChild(pos);
            pos->setKey("position");

            std::shared_ptr<cugl::JsonValue> did_shoot = cugl::JsonValue::alloc(
                static_cast<bool>(enemy->didFireBullet()));
            enemy_info->appendChild(did_shoot);
            did_shoot->setKey("did_shoot");

            std::shared_ptr<cugl::JsonValue> target_pos =
                cugl::JsonValue::allocArray();
            std::shared_ptr<cugl::JsonValue> target_pos_x =
                cugl::JsonValue::alloc(enemy->getFiredBulletDirection().x);
            std::shared_ptr<cugl::JsonValue> target_pos_y =
                cugl::JsonValue::alloc(enemy->getFiredBulletDirection().y);
            target_pos->appendChild(target_pos_x);
            target_pos->appendChild(target_pos_y);
            enemy_info->appendChild(target_pos);
            target_pos->setKey("target_pos");

            // Make sure bullet is only fired once
            enemy->clearBulletFiredState();

            std::shared_ptr<cugl::JsonValue> enemy_health =
                cugl::JsonValue::alloc(static_cast<long>(enemy->getHealth()));
            enemy_info->appendChild(enemy_health);
            enemy_health->setKey("enemy_health");

            std::shared_ptr<cugl::JsonValue> enemy_room =
                cugl::JsonValue::alloc(static_cast<long>(room_id));
            enemy_info->appendChild(enemy_room);
            enemy_room->setKey("enemy_room");

            // TODO network enemy projectiles

            // Serialize one enemy at a time to avoid reaching packet limit
            _serializer.writeSint32(5);
            _serializer.writeJson(enemy_info);

            std::vector<uint8_t> msg2 = _serializer.serialize();

            _serializer.reset();
            NetworkController::get()->send(msg2);
          }
        }
      }

      // Send all player info
      _serializer.writeSint32(NC_HOST_ALL_PLAYER_INFO);
      _serializer.writeJsonVector(player_positions);

      std::vector<uint8_t> msg = _serializer.serialize();

      _serializer.reset();
      NetworkController::get()->send(msg);
    }

    {
      // Send all timer info.
      std::shared_ptr<cugl::JsonValue> timer_info =
          cugl::JsonValue::allocObject();
      std::shared_ptr<cugl::JsonValue> millis_remaining =
          cugl::JsonValue::alloc(static_cast<long>(getMillisRemaining()));
      timer_info->appendChild(millis_remaining);
      millis_remaining->setKey("millis_remaining");

      _serializer.writeSint32(3);
      _serializer.writeJson(timer_info);
      std::vector<uint8_t> msg = _serializer.serialize();
      _serializer.reset();
      NetworkController::get()->send(msg);
    }

  } else {
    // Send just the current player information.

    std::shared_ptr<cugl::JsonValue> player_info =
        cugl::JsonValue::allocObject();

    std::shared_ptr<cugl::JsonValue> player_id = cugl::JsonValue::alloc(
        static_cast<long>(_player_controller->getMyPlayer()->getPlayerId()));
    player_info->appendChild(player_id);
    player_id->setKey("player_id");

    // send a player's set display name from itself to host
    std::shared_ptr<cugl::JsonValue> player_display_name =
        cugl::JsonValue::alloc(static_cast<std::string>(_display_name));
    player_info->appendChild(player_display_name);
    player_display_name->setKey("player_display_name");

    std::shared_ptr<cugl::JsonValue> room = cugl::JsonValue::alloc(
        static_cast<long>(_player_controller->getMyPlayer()->getRoomId()));
    player_info->appendChild(room);
    room->setKey("room");

    std::shared_ptr<cugl::JsonValue> pos = cugl::JsonValue::allocArray();
    std::shared_ptr<cugl::JsonValue> pos_x = cugl::JsonValue::alloc(
        _player_controller->getMyPlayer()->getPosition().x);
    std::shared_ptr<cugl::JsonValue> pos_y = cugl::JsonValue::alloc(
        _player_controller->getMyPlayer()->getPosition().y);
    pos->appendChild(pos_x);
    pos->appendChild(pos_y);
    player_info->appendChild(pos);
    pos->setKey("position");

    //    std::shared_ptr<cugl::JsonValue> facing_right =
    //        cugl::JsonValue::alloc(static_cast<long>(_player_controller->getMyPlayer()->getPlayerNode()->isFlipHorizontal()));
    //    player_info->appendChild(facing_right);
    //    player_info->setKey("facing_right");

    // Send individual player information.
    _serializer.writeSint32(NC_CLIENT_ONE_PLAYER_INFO);
    _serializer.writeJson(player_info);
    std::vector<uint8_t> msg = _serializer.serialize();
    _serializer.reset();
    NetworkController::get()->sendOnlyToHost(msg);
  }
}

void GameScene::sendEnemyHitNetworkInfo(int id, int room_id, int dir,
                                        float amount) {
  std::shared_ptr<cugl::JsonValue> enemy_info = cugl::JsonValue::allocObject();

  std::shared_ptr<cugl::JsonValue> enemy_id =
      cugl::JsonValue::alloc(static_cast<long>(id));
  enemy_info->appendChild(enemy_id);
  enemy_id->setKey("enemy_id");

  std::shared_ptr<cugl::JsonValue> enemy_room =
      cugl::JsonValue::alloc(static_cast<long>(room_id));
  enemy_info->appendChild(enemy_room);
  enemy_room->setKey("enemy_room");

  std::shared_ptr<cugl::JsonValue> damage_amount =
      cugl::JsonValue::alloc(amount);
  enemy_info->appendChild(damage_amount);
  damage_amount->setKey("amount");

  std::shared_ptr<cugl::JsonValue> direction =
      cugl::JsonValue::alloc(static_cast<long>(dir));
  enemy_info->appendChild(direction);
  direction->setKey("direction");

  _serializer.writeSint32(6);
  _serializer.writeJson(enemy_info);

  std::vector<uint8_t> msg = _serializer.serialize();

  _serializer.reset();
  NetworkController::get()->sendOnlyToHost(msg);
}

void GameScene::sendTerminalAddPlayerInfo(int room_id, int player_id,
                                          int num_players_req) {
  std::shared_ptr<cugl::JsonValue> terminal_info =
      cugl::JsonValue::allocObject();
  {
    std::shared_ptr<cugl::JsonValue> room_info =
        cugl::JsonValue::alloc(static_cast<long>(room_id));
    terminal_info->appendChild(room_info);
    room_info->setKey("terminal_room_id");
  }
  {
    std::shared_ptr<cugl::JsonValue> num_req_info =
        cugl::JsonValue::alloc(static_cast<long>(num_players_req));
    terminal_info->appendChild(num_req_info);
    num_req_info->setKey("num_players_req");
  }
  {
    std::shared_ptr<cugl::JsonValue> player_info =
        cugl::JsonValue::alloc(static_cast<long>(player_id));
    terminal_info->appendChild(player_info);
    player_info->setKey("player_id");
  }

  _serializer.writeSint32(NC_CLIENT_PLAYER_ADDED);
  _serializer.writeJson(terminal_info);

  std::vector<uint8_t> msg = _serializer.serialize();

  _serializer.reset();
  NetworkController::get()->sendOnlyToHost(msg);
  // Send this to host, as sendOnlyToHost doesn't send to host if it was called
  // by the host.
  if (_ishost) {
    _deserializer.receive(msg);
    std::get<Sint32>(_deserializer.read());
    _terminal_controller->processNetworkData(NC_CLIENT_PLAYER_ADDED,
                                             _deserializer.read());
    _deserializer.reset();
  }
}

void GameScene::sendBetrayalTargetInfo(int target_player_id) {
  std::shared_ptr<cugl::JsonValue> betrayal_info =
      cugl::JsonValue::allocObject();

  std::shared_ptr<cugl::JsonValue> betraying_player_info =
      cugl::JsonValue::alloc((
          static_cast<long>(_player_controller->getMyPlayer()->getPlayerId())));
  betrayal_info->appendChild(betraying_player_info);
  betraying_player_info->setKey("betraying_player_id");

  std::shared_ptr<cugl::JsonValue> target_player_info =
      cugl::JsonValue::alloc(static_cast<long>(target_player_id));
  betrayal_info->appendChild(target_player_info);
  target_player_info->setKey("target_player_id");

  _serializer.writeSint32(12);
  _serializer.writeJson(betrayal_info);

  std::vector<uint8_t> msg = _serializer.serialize();

  _serializer.reset();
  _network->sendOnlyToHost(msg);
  // Send this to host, as sendOnlyToHost doesn't send to host if it was called
  // by the host.
  if (_ishost) {
    _deserializer.receive(msg);
    std::get<Sint32>(_deserializer.read());
    processData(12, _deserializer.read());
    _deserializer.reset();
  }
}

/*
 * This simply passes on the disable message on from the host to clients for
 * now. In the future the host can do server-side logic
 */
void GameScene::sendDisablePlayerInfo(int target_player_id) {
  std::shared_ptr<cugl::JsonValue> betrayal_info =
      cugl::JsonValue::allocObject();

  std::shared_ptr<cugl::JsonValue> target_player_info =
      cugl::JsonValue::alloc(static_cast<long>(target_player_id));
  betrayal_info->appendChild(target_player_info);
  target_player_info->setKey("target_player_id");

  _serializer.writeSint32(13);
  _serializer.writeJson(betrayal_info);

  std::vector<uint8_t> msg = _serializer.serialize();

  _serializer.reset();
  _network->send(msg);
  // Send this to host, as sendOnlyToHost doesn't send to host if it was called
  // by the host.
  if (_ishost) {
    _deserializer.receive(msg);
    std::get<Sint32>(_deserializer.read());
    processData(13, _deserializer.read());
    _deserializer.reset();
  }
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
void GameScene::processData(const Sint32& code,
                            const cugl::NetworkDeserializer::Message& msg) {
  if (code == 3) {  // Timer info update
    std::shared_ptr<cugl::JsonValue> timer_info =
        std::get<std::shared_ptr<cugl::JsonValue>>(msg);
    int millis_remaining = timer_info->getInt("millis_remaining");
    setMillisRemaining(millis_remaining);
  } else if (code == 5) {  // Singular enemy update from the host
    std::shared_ptr<cugl::JsonValue> enemy =
        std::get<std::shared_ptr<cugl::JsonValue>>(msg);

    int enemy_id = enemy->getInt("enemy_id");
    int enemy_health = enemy->getInt("enemy_health");
    int enemy_room = enemy->getInt("enemy_room");

    std::shared_ptr<cugl::JsonValue> enemy_position = enemy->get("position");
    float pos_x = enemy_position->get(0)->asFloat();
    float pos_y = enemy_position->get(1)->asFloat();

    bool did_shoot = enemy->getBool("did_shoot");
    std::shared_ptr<cugl::JsonValue> target_pos = enemy->get("target_pos");
    float target_pos_x = target_pos->get(0)->asFloat();
    float target_pos_y = target_pos->get(1)->asFloat();

    updateEnemyInfo(enemy_id, enemy_room, enemy_health, pos_x, pos_y, did_shoot,
                    target_pos_x, target_pos_y);
  } else if (code == 6) {  // Enemy update from a client that damaged an enemy
    std::shared_ptr<cugl::JsonValue> enemy =
        std::get<std::shared_ptr<cugl::JsonValue>>(msg);

    int enemy_id = enemy->getInt("enemy_id");
    int enemy_room = enemy->getInt("enemy_room");
    float amount = enemy->getInt("amount");
    int direction = enemy->getInt("direction");

    std::shared_ptr<RoomModel> room =
        _level_controller->getLevelModel()->getRoom(enemy_room);

    for (std::shared_ptr<EnemyModel> enemy : room->getEnemies()) {
      if (enemy->getEnemyId() == enemy_id) {
        enemy->takeDamage(amount);
        enemy->knockback(direction);
        return;
      }
    }
  } else if (code == 12 && _ishost) {
    std::shared_ptr<cugl::JsonValue> target_data =
        std::get<std::shared_ptr<cugl::JsonValue>>(msg);

    int player_id = target_data->getInt("target_player_id");
    sendDisablePlayerInfo(player_id);
  } else if (code == 13) {
    std::shared_ptr<cugl::JsonValue> target_data =
        std::get<std::shared_ptr<cugl::JsonValue>>(msg);

    int player_id = target_data->getInt("target_player_id");

    if (player_id == _player_controller->getMyPlayer()->getPlayerId()) {
      // Does 40 damage (in total).
      _player_controller->getMyPlayer()->reduceHealth(35);
      _player_controller->getMyPlayer()->takeDamage();
    }
  }

  _deserializer.reset();
}

/**
 * Updates the health and position of the enemy with the corresponding enemy_id
 * in the room with id enemy_room;
 *
 * @param enemy_id      The enemy id.
 * @param enemy_room    The room id the enemy is in.
 * @param enemy_health  The updated enemy health.
 * @param pos_x         The updated enemy x position.
 * @param pos_y         The updated enemy y position.
 * @param did_shoot   Whether the enemy shot.
 * @param bullet_dir_x  The last shot bullet's x direction
 * @param bullet_dir_y  The last shot bullet's y direction
 */
void GameScene::updateEnemyInfo(int enemy_id, int enemy_room, int enemy_health,
                                float pos_x, float pos_y, bool did_shoot,
                                float target_pos_x, float target_pos_y) {
  std::shared_ptr<RoomModel> room =
      _level_controller->getLevelModel()->getRoom(enemy_room);

  for (std::shared_ptr<EnemyModel> enemy : room->getEnemies()) {
    if (enemy->getEnemyId() == enemy_id) {
      enemy->setPosition(pos_x, pos_y);
      enemy->setHealth(enemy_health);
      if (did_shoot) {
        enemy->addBullet(cugl::Vec2(target_pos_x, target_pos_y));
      }
      return;
    }
  }
}

void GameScene::beginContact(b2Contact* contact) {
  b2Fixture* fx1 = contact->GetFixtureA();
  b2Fixture* fx2 = contact->GetFixtureB();

  void* fx1_d = (void*)fx1->GetUserData().pointer;
  void* fx2_d = (void*)fx2->GetUserData().pointer;

  std::string fx1_name;
  if (static_cast<std::string*>(fx1_d) != nullptr)
    fx1_name.assign(*static_cast<std::string*>(fx1_d));
  std::string fx2_name;
  if (static_cast<std::string*>(fx2_d) != nullptr)
    fx2_name.assign(*static_cast<std::string*>(fx2_d));

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

    dynamic_cast<EnemyModel*>(ob1)->takeDamage(damage);
    sendEnemyHitNetworkInfo(dynamic_cast<EnemyModel*>(ob1)->getEnemyId(),
                            _player_controller->getMyPlayer()->getRoomId(),
                            _player_controller->getSword()->getMoveDir(),
                            damage);
  } else if (fx2_name == "enemy_hitbox" &&
             ob1 == _player_controller->getSword().get()) {
    float damage = 20;
    EnemyModel::EnemyType type = dynamic_cast<EnemyModel*>(ob2)->getType();
    if (type == EnemyModel::EnemyType::TURTLE) damage = 3;

    dynamic_cast<EnemyModel*>(ob2)->takeDamage(damage);
    dynamic_cast<EnemyModel*>(ob2)->knockback(
        _player_controller->getSword()->getMoveDir());
    sendEnemyHitNetworkInfo(dynamic_cast<EnemyModel*>(ob2)->getEnemyId(),
                            _player_controller->getMyPlayer()->getRoomId(),
                            _player_controller->getSword()->getMoveDir(),
                            damage);
  }

  if (fx1_name == "enemy_hitbox" &&
      ob2 == _player_controller->getMyPlayer().get()) {
    Player::State player_state = _player_controller->getMyPlayer()->getState();
    if (player_state == Player::State::DASHING) {
      dynamic_cast<EnemyModel*>(ob1)->takeDamage(5.0f);
      sendEnemyHitNetworkInfo(dynamic_cast<EnemyModel*>(ob1)->getEnemyId(),
                              _player_controller->getMyPlayer()->getRoomId(),
                              _player_controller->getMyPlayer()->getMoveDir(),
                              5.0f);
    }
  } else if (fx2_name == "enemy_hitbox" &&
             ob1 == _player_controller->getMyPlayer().get()) {
    Player::State player_state = _player_controller->getMyPlayer()->getState();
    if (player_state == Player::State::DASHING) {
      dynamic_cast<EnemyModel*>(ob2)->takeDamage(5.0f);
      sendEnemyHitNetworkInfo(dynamic_cast<EnemyModel*>(ob2)->getEnemyId(),
                              _player_controller->getMyPlayer()->getRoomId(),
                              _player_controller->getMyPlayer()->getMoveDir(),
                              5.0f);
    }
  }

  if (fx1_name == "enemy_damage" &&
      ob2 == _player_controller->getMyPlayer().get()) {
    dynamic_cast<Player*>(ob2)->takeDamage();
  } else if (fx2_name == "enemy_damage" &&
             ob1 == _player_controller->getMyPlayer().get()) {
    dynamic_cast<Player*>(ob1)->takeDamage();
  }

  if (ob1->getName() == "projectile" &&
      ob2 == _player_controller->getMyPlayer().get()) {
    dynamic_cast<Projectile*>(ob1)->setFrames(0);  // Destroy the projectile
    dynamic_cast<Player*>(ob2)->takeDamage();
  } else if (ob2->getName() == "projectile" &&
             ob1 == _player_controller->getMyPlayer().get()) {
    dynamic_cast<Player*>(ob1)->takeDamage();
    dynamic_cast<Projectile*>(ob2)->setFrames(0);  // Destroy the projectile
  }

  if (fx1_name == "enemy_hitbox" && ob2->getName() == "slash") {
    dynamic_cast<EnemyModel*>(ob1)->takeDamage();
    dynamic_cast<Projectile*>(ob2)->setFrames(0);  // Destroy the projectile
    sendEnemyHitNetworkInfo(dynamic_cast<EnemyModel*>(ob1)->getEnemyId(),
                            _player_controller->getMyPlayer()->getRoomId(),
                            _player_controller->getMyPlayer()->getMoveDir(),
                            20);
  } else if (fx2_name == "enemy_hitbox" && ob1->getName() == "slash") {
    dynamic_cast<EnemyModel*>(ob2)->takeDamage();
    dynamic_cast<Projectile*>(ob1)->setFrames(0);  // Destroy the projectile
    sendEnemyHitNetworkInfo(dynamic_cast<EnemyModel*>(ob2)->getEnemyId(),
                            _player_controller->getMyPlayer()->getRoomId(),
                            _player_controller->getMyPlayer()->getMoveDir(),
                            20);
  }

  if (fx1_name == "enemy_damage" && ob2->getName() == "Wall") {
    auto enemy = dynamic_cast<EnemyModel*>(ob1);
    if (enemy->getAttackCooldown() < 20) {
      enemy->setAttackCooldown(0);
    }
  } else if (fx2_name == "enemy_damage" && ob1->getName() == "Wall") {
    auto enemy = dynamic_cast<EnemyModel*>(ob2);
    if (enemy->getAttackCooldown() < 20) {
      enemy->setAttackCooldown(0);
    }
  }

  if (ob1->getName() == "projectile" &&
      ob2 == _player_controller->getSword().get()) {
    dynamic_cast<Projectile*>(ob1)->setFrames(0);  // Destroy the projectile
  } else if (ob2->getName() == "projectile" &&
             ob1 == _player_controller->getSword().get()) {
    dynamic_cast<Projectile*>(ob2)->setFrames(0);  // Destroy the projectile
  }

  if ((ob1->getName() == "projectile" || ob1->getName() == "slash") &&
      ob2->getName() == "Wall") {
    dynamic_cast<Projectile*>(ob1)->setFrames(0);  // Destroy the projectile
  } else if ((ob2->getName() == "projectile" || ob1->getName() == "slash") &&
             ob1->getName() == "Wall") {
    dynamic_cast<Projectile*>(ob2)->setFrames(0);  // Destroy the projectile
  }

  if (fx1_name.find("door") != std::string::npos &&
      ob2 == _player_controller->getMyPlayer().get()) {
    _level_controller->changeRoom(fx1_name);
  } else if (fx2_name.find("door") != std::string::npos &&
             ob1 == _player_controller->getMyPlayer().get()) {
    _level_controller->changeRoom(fx2_name);
  }

  if (fx1_name == "terminal_range" &&
      ob2 == _player_controller->getMyPlayer().get()) {
    if (!dynamic_cast<TerminalSensor*>(ob1)->isActivated()) {
      std::shared_ptr<RoomModel> room =
          _level_controller->getLevelModel()->getCurrentRoom();

      _terminal_controller->setActive(room->getKey(),
                                      room->getNumPlayersRequired(),
                                      dynamic_cast<TerminalSensor*>(ob1));

      sendTerminalAddPlayerInfo(
          room->getKey(), _player_controller->getMyPlayer()->getPlayerId(),
          room->getNumPlayersRequired());
    }
  } else if (fx2_name == "terminal_range" &&
             ob1 == _player_controller->getMyPlayer().get()) {
    if (!dynamic_cast<TerminalSensor*>(ob2)->isActivated()) {
      std::shared_ptr<RoomModel> room =
          _level_controller->getLevelModel()->getCurrentRoom();

      _terminal_controller->setActive(room->getKey(),
                                      room->getNumPlayersRequired(),
                                      dynamic_cast<TerminalSensor*>(ob2));

      sendTerminalAddPlayerInfo(
          room->getKey(), _player_controller->getMyPlayer()->getPlayerId(),
          room->getNumPlayersRequired());
    }
  }
}

/**
 * Checks that the network connection is still active.
 *
 * Even if you are not sending messages all that often, you need to be calling
 * this method regularly. This method is used to determine the current state
 * of the scene.
 *
 * @return true if the network connection is still active.
 */
bool GameScene::checkConnection() {
  switch (_network->getStatus()) {
    case cugl::NetworkConnection::NetStatus::Pending:
    case cugl::NetworkConnection::NetStatus::Connected:
    case cugl::NetworkConnection::NetStatus::Reconnecting:
      break;
    case cugl::NetworkConnection::NetStatus::RoomNotFound:
    case cugl::NetworkConnection::NetStatus::ApiMismatch:
    case cugl::NetworkConnection::NetStatus::GenericError:
    case cugl::NetworkConnection::NetStatus::Disconnected:
      disconnect();
      return false;
  }
  return true;
}

void GameScene::beforeSolve(b2Contact* contact, const b2Manifold* oldManifold) {
}

void GameScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch) {
  Scene2::render(batch);
}

void GameScene::updateCamera(float timestep) {
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

void GameScene::updateMillisRemainingIfHost() {
  if (_ishost) {
    cugl::Timestamp stamp = cugl::Timestamp();
    int milli_difference =
        cugl::Timestamp::ellapsedMillis(_last_timestamp, stamp);
    _millis_remaining -= milli_difference;
    _last_timestamp = stamp;
  }

  // TODO if milliseconds reaches 0 - need to activate betrayer win condition
  // (for host or for everyone?)
}

std::string GameScene::getTimerString() {
  int total_seconds = getMillisRemaining() / 1000;
  int minutes = total_seconds / 60;
  int seconds = total_seconds % 60;

  // append leading 0s if numbers are below 10
  std::string minute_string = cugl::strtool::format("%d:", minutes);
  if (minutes < 10) {
    minute_string = "0" + minute_string;
  }
  std::string second_string = cugl::strtool::format("%d", seconds);
  if (seconds < 10) {
    second_string = "0" + second_string;
  }

  return minute_string + second_string;
}
