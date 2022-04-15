#include "GameScene.h"

#include <box2d/b2_collision.h>
#include <box2d/b2_contact.h>
#include <cugl/cugl.h>

#include "../controllers/actions/Attack.h"
#include "../controllers/actions/Dash.h"
#include "../controllers/actions/Movement.h"
#include "../controllers/actions/OpenMap.h"
#include "../controllers/actions/TargetPlayer.h"
#include "../loaders/CustomScene2Loader.h"
#include "../models/RoomModel.h"
#include "../models/tiles/Wall.h"

#define SCENE_HEIGHT 720
#define CAMERA_SMOOTH_SPEED 2.0f

bool GameScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets,
    const std::shared_ptr<level_gen::LevelGenerator>& level_gen,
    bool is_betrayer) {
  if (_active) return false;
  _active = true;

  // Initialize the scene to a locked width.

  cugl::Size dim = cugl::Application::get()->getDisplaySize();
  dim *= SCENE_HEIGHT / ((dim.width > dim.height) ? dim.width : dim.height);

  if (assets == nullptr || !cugl::Scene2::init(dim)) return false;

  _assets = assets;

  _world_node = _assets->get<cugl::scene2::SceneNode>("world-scene");
  _world_node->setContentSize(dim);
  
  std::shared_ptr<cugl::Texture> target_texture = _assets->get<cugl::Texture>("target-player");
  auto target_icon_node = cugl::scene2::SpriteNode::alloc(target_texture, 1, 1);
  target_icon_node->setName("target-icon");
  target_icon_node->setVisible(false);
  _world_node->addChild(target_icon_node);

  _debug_node = cugl::scene2::SceneNode::alloc();
  _debug_node->setContentSize(dim);

  _level_controller =
      LevelController::alloc(_assets, _world_node, _debug_node, level_gen);
  _controllers.push_back(_level_controller->getHook());

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

  populate(dim);

  _world_node->doLayout();

  auto terminal_voting_layer =
      assets->get<cugl::scene2::SceneNode>("terminal-voting-scene");
  terminal_voting_layer->setContentSize(dim);
  terminal_voting_layer->doLayout();

  _terminal_controller = TerminalController::alloc(_assets);
  _controllers.push_back(_terminal_controller);


  auto background_layer = assets->get<cugl::scene2::SceneNode>("background");
  background_layer->setContentSize(dim);
  background_layer->doLayout();

  auto ui_layer = assets->get<cugl::scene2::SceneNode>("ui-scene");
  ui_layer->setContentSize(dim);
  ui_layer->doLayout();

  auto win_layer = assets->get<cugl::scene2::SceneNode>("win-scene");
  win_layer->setContentSize(dim);
  win_layer->doLayout();
  win_layer->setVisible(false);
  
  auto target_player_button = ui_layer->getChildByName<cugl::scene2::Button>("target-player");
  target_player_button->setVisible(is_betrayer);

  auto timer_text = ui_layer->getChildByName<cugl::scene2::Label>("timer");
  std::string timer_msg = getTimerString();
  timer_text->setText(timer_msg);
  timer_text->setForeground(cugl::Color4::BLACK);

  _num_terminals_activated = 0;
  _num_terminals_corrupted = 0;
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

  auto name_text = ui_layer->getChildByName<cugl::scene2::Label>("name");
  std::string name_msg =  cugl::strtool::format("player %d", _my_player->getPlayerId());
  name_text->setText(name_msg);
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

  cugl::Scene2::addChild(background_layer);
  cugl::Scene2::addChild(_world_node);
  cugl::Scene2::addChild(_map);
  cugl::Scene2::addChild(ui_layer);
  cugl::Scene2::addChild(terminal_voting_layer);
  cugl::Scene2::addChild(win_layer);
  cugl::Scene2::addChild(_debug_node);
  _debug_node->setVisible(false);

  InputController::get()->init(_assets, cugl::Scene2::getBounds());
  InputController::get<TargetPlayer>()->setActive(is_betrayer);

  setMillisRemaining(900000);

  return true;
}

void GameScene::dispose() {
  if (!_active) return;
  InputController::get()->dispose();
  _active = false;
}

void GameScene::populate(cugl::Size dim) {
  // Initialize the player with texture and size, then add to world.
  std::shared_ptr<cugl::Texture> player = _assets->get<cugl::Texture>("player");

  _my_player = Player::alloc(cugl::Vec2::ZERO, "Johnathan");
  _my_player->setBetrayer(_is_betrayer);
  _players.push_back(_my_player);
  _level_controller->getLevelModel()->setPlayer(_my_player);

  auto player_node = cugl::scene2::SpriteNode::alloc(player, 9, 10);
  _my_player->setPlayerNode(player_node);
  _world_node->addChild(player_node);
  _world->addObstacle(_my_player);

  _player_controller = PlayerController::alloc(_my_player, _assets, _world,
                                               _world_node, _debug_node);

  _sword = Sword::alloc(dim / 2.0f);
  _world->addObstacle(_sword);
  _sword->setEnabled(false);

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
  _my_player->setDebugScene(_debug_node);
  _my_player->setDebugColor(cugl::Color4f::BLACK);
  _sword->setDebugScene(_debug_node);
  _sword->setDebugColor(cugl::Color4f::BLACK);

  std::shared_ptr<RoomModel> current_room =
      _level_controller->getLevelModel()->getCurrentRoom();
  _my_player->setRoomId(current_room->getKey());
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

    _network->receive(
        [this](const std::vector<uint8_t>& data) { processData(data); });
    checkConnection();
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

  if (InputController::get<OpenMap>()->didOpenMap()) {
    _map->setVisible(!_map->isVisible());
  }
  
  auto target_player = InputController::get<TargetPlayer>();
  if (target_player->didChangeTarget()) {
    int current_room_id = _my_player->getRoomId();
    bool found_player = false;
    bool others_in_room = false;
    int first_found_player = -1;
    for (std::shared_ptr<Player> player : _players) {
      if (player->getRoomId() == current_room_id && player->getPlayerId() != _my_player->getPlayerId()) {
        others_in_room = true;
        if (first_found_player == -1) {
          first_found_player = player->getPlayerId();
        }
        if (target_player->getTarget() == -1 || !target_player->hasSeenPlayerId(player->getPlayerId())) {
          // Sets the target to a fresh player
          target_player->setTarget(player->getPlayerId());
          auto target_icon_node = _world_node->getChildByName<cugl::scene2::SceneNode>("target-icon");
          target_icon_node->setPosition(player->getPlayerNode()->getPosition());
          target_icon_node->setVisible(true);
          found_player = true;
          break;
        }
      }
    }
    // Cycled through all players and they have all already been visited, go back to the first
    if (!found_player && others_in_room && first_found_player != -1) {
      target_player->clearDirtyPlayers();
      target_player->setTarget(first_found_player);
    }
  }
  
  auto target_icon_node = _world_node->getChildByName<cugl::scene2::SceneNode>("target-icon");
  if (target_player->getTarget() != -1) {
    for (std::shared_ptr<Player> player : _players) {
      if (player->getRoomId() == _my_player->getRoomId() && player->getPlayerId() == target_player->getTarget()) {
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

  // Movement
  _player_controller->update(
      timestep, InputController::get<Movement>()->getMovement(),
      InputController::get<Attack>()->isAttacking(),
      InputController::get<Dash>()->isDashing(),
      InputController::get<Attack>()->holdAttack(), _sword);

  std::shared_ptr<RoomModel> current_room =
      _level_controller->getLevelModel()->getCurrentRoom();
  int room_id = current_room->getKey();
  _my_player->setRoomId(current_room->getKey());
  for (std::shared_ptr<EnemyModel>& enemy : current_room->getEnemies()) {
    switch (enemy->getType()) {
      case EnemyModel::GRUNT: {
        _grunt_controller->update(timestep, enemy, _players, room_id);
        break;
      }
      case EnemyModel::SHOTGUNNER: {
        _shotgunner_controller->update(timestep, enemy, _players, room_id);
        break;
      }
      case EnemyModel::TANK: {
        _tank_controller->update(timestep, enemy, _players, room_id);
        break;
      }
      case EnemyModel::TURTLE: {
        _turtle_controller->update(timestep, enemy, _players, room_id);
        break;
      }
    }
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
  std::string name_msg =
      cugl::strtool::format("player %d", _my_player->getPlayerId());
  name_text->setText(name_msg);
  name_text->setForeground(cugl::Color4::BLACK);

  auto activated_text = ui_layer->getChildByName<cugl::scene2::Label>("activated_num");
  std::string activated_msg = cugl::strtool::format(std::to_string(_num_terminals_activated));
  activated_text->setText(activated_msg);

  auto corrupted_text = ui_layer->getChildByName<cugl::scene2::Label>("corrupted_num");
  std::string corrupted_msg = cugl::strtool::format(std::to_string(_num_terminals_corrupted));
  corrupted_text->setText(corrupted_msg);


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
      if (enemy->getPromiseToChangePhysics())
        enemy->setEnabled(enemy->getPromiseToEnable());
      ++it;
    }
  }
  _my_player->checkDeleteSlashes(_world, _world_node);
}

void GameScene::sendNetworkInfo() {
  if (auto player_id = _network->getPlayerID()) {
    _my_player->setPlayerId(*player_id);
  }
  if (_ishost) {
    {
      std::vector<std::shared_ptr<cugl::JsonValue>> player_positions;
      std::set<int> rooms_checked_for_enemies;

      for (std::shared_ptr<Player> player : _players) {
        // get player info

        std::shared_ptr<cugl::JsonValue> player_info =
            cugl::JsonValue::allocObject();

        std::shared_ptr<cugl::JsonValue> player_id =
            cugl::JsonValue::alloc(static_cast<long>(player->getPlayerId()));
        player_info->appendChild(player_id);
        player_id->setKey("player_id");

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

        // get enemy info only for the rooms that players are in
        for (std::shared_ptr<EnemyModel> enemy : player_room->getEnemies()) {
          std::shared_ptr<cugl::JsonValue> enemy_info =
              cugl::JsonValue::allocObject();

          std::shared_ptr<cugl::JsonValue> enemy_id =
              cugl::JsonValue::alloc(static_cast<long>(enemy->getEnemyId()));
          enemy_info->appendChild(enemy_id);
          enemy_id->setKey("enemy_id");

          std::shared_ptr<cugl::JsonValue> pos = cugl::JsonValue::allocArray();
          std::shared_ptr<cugl::JsonValue> pos_x =
              cugl::JsonValue::alloc(enemy->getPosition().x);
          std::shared_ptr<cugl::JsonValue> pos_y =
              cugl::JsonValue::alloc(enemy->getPosition().y);
          pos->appendChild(pos_x);
          pos->appendChild(pos_y);
          enemy_info->appendChild(pos);
          pos->setKey("position");

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
          _network->send(msg2);
        }
      }

      // Send all player info
      _serializer.writeSint32(2);
      _serializer.writeJsonVector(player_positions);

      std::vector<uint8_t> msg = _serializer.serialize();

      _serializer.reset();
      _network->send(msg);
    }

    // ======= SEND TERMINAL VOTING INFO ==========
    {
      std::unordered_map<int, TerminalController::VotingInfo> voting_info =
          _terminal_controller->getVotingInfo();

      for (auto it = voting_info.begin(); it != voting_info.end(); ++it) {
        auto info = cugl::JsonValue::allocObject();

        auto terminal_room_id_info = cugl::JsonValue::alloc(
            static_cast<long>((it->second).terminal_room_id));
        info->appendChild(terminal_room_id_info);
        terminal_room_id_info->setKey("terminal_room_id");

        auto players_ids_info = cugl::JsonValue::allocArray();
        for (int player_id : (it->second).players) {
          auto player_id_info =
              cugl::JsonValue::alloc(static_cast<long>(player_id));
          players_ids_info->appendChild(player_id_info);
        }
        info->appendChild(players_ids_info);
        players_ids_info->setKey("players");

        _serializer.writeSint32(8);
        _serializer.writeJson(info);

        std::vector<uint8_t> msg = _serializer.serialize();
        _serializer.reset();
        _network->send(msg);
      }
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
      std::vector<uint8_t> timer_msg = _serializer.serialize();
      _serializer.reset();
      _network->send(timer_msg);
    }

  } else {
    // Send just the current player information.

    std::shared_ptr<cugl::JsonValue> player_info =
        cugl::JsonValue::allocObject();

    std::shared_ptr<cugl::JsonValue> player_id =
        cugl::JsonValue::alloc(static_cast<long>(_my_player->getPlayerId()));
    player_info->appendChild(player_id);
    player_id->setKey("player_id");
    
    std::shared_ptr<cugl::JsonValue> room =
        cugl::JsonValue::alloc(static_cast<long>(_my_player->getRoomId()));
    player_info->appendChild(room);
    room->setKey("room");

    std::shared_ptr<cugl::JsonValue> pos = cugl::JsonValue::allocArray();
    std::shared_ptr<cugl::JsonValue> pos_x =
        cugl::JsonValue::alloc(_my_player->getPosition().x);
    std::shared_ptr<cugl::JsonValue> pos_y =
        cugl::JsonValue::alloc(_my_player->getPosition().y);
    pos->appendChild(pos_x);
    pos->appendChild(pos_y);
    player_info->appendChild(pos);
    pos->setKey("position");

    //    std::shared_ptr<cugl::JsonValue> facing_right =
    //        cugl::JsonValue::alloc(static_cast<long>(_my_player->getPlayerNode()->isFlipHorizontal()));
    //    player_info->appendChild(facing_right);
    //    player_info->setKey("facing_right");

    // Send individual player information.
    _serializer.writeSint32(4);
    _serializer.writeJson(player_info);
    std::vector<uint8_t> msg = _serializer.serialize();
    _serializer.reset();
    _network->sendOnlyToHost(msg);
  }
}

void GameScene::sendEnemyHitNetworkInfo(int id, int room_id) {
  std::shared_ptr<cugl::JsonValue> enemy_info = cugl::JsonValue::allocObject();

  std::shared_ptr<cugl::JsonValue> enemy_id =
      cugl::JsonValue::alloc(static_cast<long>(id));
  enemy_info->appendChild(enemy_id);
  enemy_id->setKey("enemy_id");

  std::shared_ptr<cugl::JsonValue> enemy_room =
      cugl::JsonValue::alloc(static_cast<long>(room_id));
  enemy_info->appendChild(enemy_room);
  enemy_room->setKey("enemy_room");

  _serializer.writeSint32(6);
  _serializer.writeJson(enemy_info);

  std::vector<uint8_t> msg = _serializer.serialize();

  _serializer.reset();
  _network->sendOnlyToHost(msg);
}

void GameScene::sendTerminalAddPlayerInfo(int room_id, int player_id) {
  std::shared_ptr<cugl::JsonValue> terminal_info =
      cugl::JsonValue::allocObject();

  std::shared_ptr<cugl::JsonValue> room_info =
      cugl::JsonValue::alloc(static_cast<long>(room_id));
  terminal_info->appendChild(room_info);
  room_info->setKey("terminal_room_id");

  std::shared_ptr<cugl::JsonValue> player_info =
      cugl::JsonValue::alloc(static_cast<long>(player_id));
  terminal_info->appendChild(player_info);
  player_info->setKey("player_id");

  _serializer.writeSint32(7);
  _serializer.writeJson(terminal_info);

  std::vector<uint8_t> msg = _serializer.serialize();

  _serializer.reset();
  _network->sendOnlyToHost(msg);
  // Send this to host, as sendOnlyToHost doesn't send to host if it was called
  // by the host.
  if (_ishost) processData(msg);
}

void GameScene::sendBetrayalTargetInfo(int target_player_id) {
  std::shared_ptr<cugl::JsonValue> betrayal_info =
      cugl::JsonValue::allocObject();

  std::shared_ptr<cugl::JsonValue> betraying_player_info =
      cugl::JsonValue::alloc((static_cast<long>(_my_player->getPlayerId())));
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
  if (_ishost) processData(msg);
}

/*
 * This simply passes on the disable message on from the host to clients for now.
 * In the future the host can do server-side logic
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
  if (_ishost) processData(msg);
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
void GameScene::processData(const std::vector<uint8_t>& data) {
  _deserializer.receive(data);
  Sint32 code = std::get<Sint32>(_deserializer.read());
  if (code == 2) {  // All player info update
    cugl::NetworkDeserializer::Message player_msg = _deserializer.read();

    std::vector<std::shared_ptr<cugl::JsonValue>> player_positions =
        std::get<std::vector<std::shared_ptr<cugl::JsonValue>>>(player_msg);
    for (std::shared_ptr<cugl::JsonValue> player : player_positions) {
      int player_id = player->getInt("player_id");
      int room_id = player->getInt("room");
      std::shared_ptr<cugl::JsonValue> player_position =
          player->get("position");
      float pos_x = player_position->get(0)->asFloat();
      float pos_y = player_position->get(1)->asFloat();
      updatePlayerInfo(player_id, room_id, pos_x, pos_y);
    }
  } else if (code == 3) {  // Timer info update
    cugl::NetworkDeserializer::Message timer_msg = _deserializer.read();
    std::shared_ptr<cugl::JsonValue> timer_info =
        std::get<std::shared_ptr<cugl::JsonValue>>(timer_msg);
    int millis_remaining = timer_info->getInt("millis_remaining");
    setMillisRemaining(millis_remaining);
  } else if (code == 4) {  // Single player info update
    cugl::NetworkDeserializer::Message msg = _deserializer.read();
    std::shared_ptr<cugl::JsonValue> player =
        std::get<std::shared_ptr<cugl::JsonValue>>(msg);
    int player_id = player->getInt("player_id");
    int room_id = player->getInt("room");
    std::shared_ptr<cugl::JsonValue> player_position = player->get("position");
    float pos_x = player_position->get(0)->asFloat();
    float pos_y = player_position->get(1)->asFloat();
    updatePlayerInfo(player_id, room_id, pos_x, pos_y);
  } else if (code == 5) {  // Singular enemy update from the host
    cugl::NetworkDeserializer::Message enemy_msg = _deserializer.read();

    std::shared_ptr<cugl::JsonValue> enemy =
        std::get<std::shared_ptr<cugl::JsonValue>>(enemy_msg);

    int enemy_id = enemy->getInt("enemy_id");
    int enemy_health = enemy->getInt("enemy_health");
    int enemy_room = enemy->getInt("enemy_room");
    std::shared_ptr<cugl::JsonValue> enemy_position = enemy->get("position");
    float pos_x = enemy_position->get(0)->asFloat();
    float pos_y = enemy_position->get(1)->asFloat();
    updateEnemyInfo(enemy_id, enemy_room, enemy_health, pos_x, pos_y);
  } else if (code == 6) {  // Enemy update from a client that damaged an enemy
    cugl::NetworkDeserializer::Message enemy_msg = _deserializer.read();

    std::shared_ptr<cugl::JsonValue> enemy =
        std::get<std::shared_ptr<cugl::JsonValue>>(enemy_msg);

    int enemy_id = enemy->getInt("enemy_id");
    int enemy_room = enemy->getInt("enemy_room");

    std::shared_ptr<RoomModel> room =
        _level_controller->getLevelModel()->getRoom(enemy_room);

    for (std::shared_ptr<EnemyModel> enemy : room->getEnemies()) {
      if (enemy->getEnemyId() == enemy_id) {
        enemy->takeDamage();
        return;
      }
    }
  } else if (code == 7) {  // Receive one player added to terminal from client.
    cugl::NetworkDeserializer::Message msg = _deserializer.read();
    _terminal_controller->processNetworkData(code, msg);
  } else if (code == 8 && !_ishost) {  // Receive voting info from host.
    cugl::NetworkDeserializer::Message msg = _deserializer.read();
    _terminal_controller->processNetworkData(code, msg);
  }
  
  else if (code == 12 && _ishost) {
    cugl::NetworkDeserializer::Message betrayal_target_msg = _deserializer.read();

    std::shared_ptr<cugl::JsonValue> target_data =
        std::get<std::shared_ptr<cugl::JsonValue>>(betrayal_target_msg);

    int player_id = target_data->getInt("target_player_id");
    sendDisablePlayerInfo(player_id);
  } else if (code == 13) {
    cugl::NetworkDeserializer::Message betrayal_target_msg = _deserializer.read();

    std::shared_ptr<cugl::JsonValue> target_data =
        std::get<std::shared_ptr<cugl::JsonValue>>(betrayal_target_msg);

    int player_id = target_data->getInt("target_player_id");
    
    if (player_id == _my_player->getPlayerId()) {
      // Does 40 damage (in total).
      _my_player->reduceHealth(35);
      _my_player->takeDamage();
    }
  }

  _deserializer.reset();
}

/**
 * Updates the position of the player with the corresponding player_id in the
 * _players list.
 *
 * @param player_id The player ids
 * @param pos_x The updated player x position
 * @param pos_y The updated player y position
 */
void GameScene::updatePlayerInfo(int player_id, int room_id, float pos_x, float pos_y) {
  if (player_id == _my_player->getPlayerId()) {
    return;
  }
  for (std::shared_ptr<Player> player : _players) {
    if (player->getPlayerId() == player_id) {
      cugl::Vec2 old_position = player->getPosition();

      // Movement must exceed this value to be animated
      const float MOVEMENT_THRESH = 1;
      if (abs(pos_x - old_position.x) > MOVEMENT_THRESH ||
          abs(pos_y - old_position.y) > MOVEMENT_THRESH) {
        player->setState(Player::MOVING);
      } else {
        player->setState(Player::IDLE);
      }
      player->setRoomId(room_id);
      player->setPosition(pos_x, pos_y);
      player->animate(pos_x - old_position.x, pos_y - old_position.y);
      return;
    }
  }
  // Haven't found a player with the player_id, so we must create a new one

  // Initialize the player with texture and size, then add to world.
  cugl::Size dim = cugl::Application::get()->getDisplaySize();
  dim *= SCENE_HEIGHT / ((dim.width > dim.height) ? dim.width : dim.height);

  std::shared_ptr<cugl::Texture> player = _assets->get<cugl::Texture>("player");

  std::shared_ptr<Player> new_player =
      Player::alloc(dim + cugl::Vec2(20, 20), "Johnathan");
  new_player->setPlayerId(player_id);
  _players.push_back(new_player);

  auto player_node = cugl::scene2::SpriteNode::alloc(player, 9, 10);
  new_player->setPlayerNode(player_node);
  _world_node->addChild(player_node);
  _world->addObstacle(new_player);

  new_player->setDebugScene(_debug_node);
  new_player->setDebugColor(cugl::Color4(cugl::Color4::BLACK));
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
 */
void GameScene::updateEnemyInfo(int enemy_id, int enemy_room, int enemy_health,
                                float pos_x, float pos_y) {
  std::shared_ptr<RoomModel> room =
      _level_controller->getLevelModel()->getRoom(enemy_room);

  for (std::shared_ptr<EnemyModel> enemy : room->getEnemies()) {
    if (enemy->getEnemyId() == enemy_id) {
      enemy->setPosition(pos_x, pos_y);
      enemy->setHealth(enemy_health);
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

  if (fx1_name == "enemy_hitbox" && ob2 == _sword.get()) {
    dynamic_cast<EnemyModel*>(ob1)->takeDamage();
    sendEnemyHitNetworkInfo(dynamic_cast<EnemyModel*>(ob1)->getEnemyId(),
                            _my_player->getRoomId());
  } else if (fx2_name == "enemy_hitbox" && ob1 == _sword.get()) {
    dynamic_cast<EnemyModel*>(ob2)->takeDamage();
    sendEnemyHitNetworkInfo(dynamic_cast<EnemyModel*>(ob2)->getEnemyId(),
                            _my_player->getRoomId());
  }

  if (fx1_name == "enemy_damage" && ob2 == _my_player.get()) {
    dynamic_cast<Player*>(ob2)->takeDamage();
  } else if (fx2_name == "enemy_damage" && ob1 == _my_player.get()) {
    dynamic_cast<Player*>(ob1)->takeDamage();
  }

  if (ob1->getName() == "projectile" && ob2 == _my_player.get()) {
    dynamic_cast<Projectile*>(ob1)->setFrames(0);  // Destroy the projectile
    dynamic_cast<Player*>(ob2)->takeDamage();
  } else if (ob2->getName() == "projectile" && ob1 == _my_player.get()) {
    dynamic_cast<Player*>(ob1)->takeDamage();
    dynamic_cast<Projectile*>(ob2)->setFrames(0);  // Destroy the projectile
  }

  if (fx1_name == "enemy_hitbox" && ob2->getName() == "slash") {
    dynamic_cast<EnemyModel*>(ob1)->takeDamage();
    dynamic_cast<Projectile*>(ob2)->setFrames(0);  // Destroy the projectile
    sendEnemyHitNetworkInfo(dynamic_cast<EnemyModel*>(ob1)->getEnemyId(),
                            _my_player->getRoomId());
  } else if (fx2_name == "enemy_hitbox" && ob1->getName() == "slash") {
    dynamic_cast<EnemyModel*>(ob2)->takeDamage();
    dynamic_cast<Projectile*>(ob1)->setFrames(0);  // Destroy the projectile
    sendEnemyHitNetworkInfo(dynamic_cast<EnemyModel*>(ob2)->getEnemyId(),
                            _my_player->getRoomId());
  }

  if (ob1->getName() == "projectile" && ob2 == _sword.get()) {
    dynamic_cast<Projectile*>(ob1)->setFrames(0);  // Destroy the projectile
  } else if (ob2->getName() == "projectile" && ob1 == _sword.get()) {
    dynamic_cast<Projectile*>(ob2)->setFrames(0);  // Destroy the projectile
  }

  if ((ob1->getName() == "projectile" || ob1->getName() == "slash") &&
      ob2->getName() == "Wall") {
    dynamic_cast<Projectile*>(ob1)->setFrames(0);  // Destroy the projectile
  } else if ((ob2->getName() == "projectile" || ob1->getName() == "slash") &&
             ob1->getName() == "Wall") {
    dynamic_cast<Projectile*>(ob2)->setFrames(0);  // Destroy the projectile
  }

  if (fx1_name.find("door") != std::string::npos && ob2 == _my_player.get()) {
    _level_controller->changeRoom(fx1_name);
  } else if (fx2_name.find("door") != std::string::npos &&
             ob1 == _my_player.get()) {
    _level_controller->changeRoom(fx2_name);
  }

  if (fx1_name == "terminal_range" && ob2 == _my_player.get()) {
    if (!dynamic_cast<TerminalSensor*>(ob1)->isActivated()) {
      std::shared_ptr<RoomModel> room =
          _level_controller->getLevelModel()->getCurrentRoom();
      _terminal_controller->setActive(room->getKey(),
                                      room->getNumPlayersRequired());

      sendTerminalAddPlayerInfo(room->getKey(), _my_player->getPlayerId());

      dynamic_cast<TerminalSensor*>(ob1)->activate();
      if (!_is_betrayer) {
        _num_terminals_activated += 1;
      } else {
        _num_terminals_corrupted += 1;
      }
    }
  } else if (fx2_name == "terminal_range" && ob1 == _my_player.get()) {
    if (!dynamic_cast<TerminalSensor*>(ob2)->isActivated()) {
      std::shared_ptr<RoomModel> room =
          _level_controller->getLevelModel()->getCurrentRoom();
      _terminal_controller->setActive(room->getKey(),
                                      room->getNumPlayersRequired());

      sendTerminalAddPlayerInfo(room->getKey(), _my_player->getPlayerId());

      dynamic_cast<TerminalSensor*>(ob2)->activate();
      if (!_is_betrayer) {
        _num_terminals_activated += 1;
      } else {
        _num_terminals_corrupted += 1;
      }
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
      _world_node->getSize() / 2.0f - _my_player->getPosition();

  cugl::Vec2 smoothed_position;
  cugl::Vec2::lerp(_world_node->getPosition(), desired_position,
                   CAMERA_SMOOTH_SPEED * timestep, &smoothed_position);

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
