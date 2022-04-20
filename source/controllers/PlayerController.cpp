#include "PlayerController.h"

#include "CollisionFiltering.h"
#include "NetworkController.h"
#include "actions/Attack.h"
#include "actions/Dash.h"
#include "actions/Movement.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 100

#define HOLD_ATTACK_COLOR_LIMIT 5
#define HOLD_ATTACK_COUNT 1000 /* milliseconds */
#define ATTACK_FRAMES 25
#define ATTACK_STOP_MOVEMENT 10
#define HURT_FRAMES 10
#define DEAD_FRAMES 175
// MAX_LIVE_FRAMES in projectile.cpp MUST be SLASH_FRAMES * 6
#define SLASH_FRAMES 7

#define HEALTH 100

#define MIN_POS_CHANGE 0.005

// The max number of milliseconds between player network position updates.
#define PLAYER_NETWORK_POS_UPDATE_MAX 100.0f

#pragma mark PlayerController

PlayerController::PlayerController(){};

bool PlayerController::init(
    const std::shared_ptr<Player>& player,
    const std::shared_ptr<cugl::AssetManager>& assets,
    const std::shared_ptr<cugl::physics2::ObstacleWorld>& world,
    const std::shared_ptr<cugl::scene2::SceneNode>& world_node,
    const std::shared_ptr<cugl::scene2::SceneNode>& debug_node) {
  _player = player;
  _assets = assets;
  _slash_texture = _assets->get<cugl::Texture>("energy-slash");
  _world = world;
  _world_node = world_node;
  _debug_node = debug_node;

  _sword = Sword::alloc(cugl::Vec2::ZERO);
  _world->addObstacle(_sword);
  _sword->setEnabled(false);
  _sword->setDebugScene(_debug_node);
  _sword->setDebugColor(cugl::Color4f::BLACK);

  NetworkController::get()->addListener(
      [=](const Sint32& code, const cugl::NetworkDeserializer::Message& msg) {
        this->processData(code, msg);
      });

  return true;
}

void PlayerController::update(float timestep) {
  for (auto it : _players) {
    if (it.first != _player->getPlayerId()) interpolate(timestep, it.second);
  }

  move(timestep);
  attack();
  updateSlashes(timestep);

  if (_player->_hurt_frames == 0) {
    _player->getPlayerNode()->setColor(cugl::Color4::WHITE);
  }
  _player->_hurt_frames--;

  // CHECK IF RAN OUT OF HEALTH
  if (_player->getHealth() <= 0 && !_player->getDead()) {
    _player->dies();
  }

  // CHECK IF HAS BEEN DEAD FOR LONG ENOUGH TO REVIVE
  if (_player->getDead() && _player->_hurt_frames <= 0) {
    _player->setHealth(HEALTH);
    _player->setDead(false);
    _player->setRespawning(true);
  }

  // Animate the player
  _player->animate();
}

void PlayerController::processData(
    const Sint32& code, const cugl::NetworkDeserializer::Message& msg) {
  switch (code) {
    case NC_HOST_ALL_PLAYER_INFO: {
      std::vector<std::shared_ptr<cugl::JsonValue>> player_positions =
          std::get<std::vector<std::shared_ptr<cugl::JsonValue>>>(msg);
      for (std::shared_ptr<cugl::JsonValue> player : player_positions) {
        int player_id = player->getInt("player_id");
        string player_display_name = player->getString("player_display_name");
        int room_id = player->getInt("room");
        std::shared_ptr<cugl::JsonValue> player_position =
            player->get("position");
        float pos_x = player_position->get(0)->asFloat();
        float pos_y = player_position->get(1)->asFloat();
        processPlayerInfo(player_id, room_id, pos_x, pos_y,
                          player_display_name);
      }
    } break;
    case NC_CLIENT_ONE_PLAYER_INFO: {
      std::shared_ptr<cugl::JsonValue> player =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);
      int player_id = player->getInt("player_id");
      string player_display_name = player->getString("player_display_name");
      int room_id = player->getInt("room");
      std::shared_ptr<cugl::JsonValue> player_position =
          player->get("position");
      float pos_x = player_position->get(0)->asFloat();
      float pos_y = player_position->get(1)->asFloat();
      processPlayerInfo(player_id, room_id, pos_x, pos_y, player_display_name);
    } break;
    default:
      break;
  }
}

void PlayerController::processPlayerInfo(int player_id, int room_id,
                                         float pos_x, float pos_y,
                                         string player_display_name) {
  if (player_id == _player->getPlayerId()) return;

  if (getPlayer(player_id) == nullptr) {
    // Haven't found a player with the player_id, so we must create a new one

    std::shared_ptr<cugl::Texture> player =
        _assets->get<cugl::Texture>("player");

    cugl::Vec2 pos = ((cugl::Vec2)_world_node->getContentSize()) / 2.0f;
    std::shared_ptr<Player> new_player = Player::alloc(
        pos + cugl::Vec2(20, 20), "Johnathan", player_display_name);
    new_player->setSensor(true);  // Makes it so we don't move other players
    new_player->setPlayerId(player_id);
    addPlayer(new_player);

    auto player_node = cugl::scene2::SpriteNode::alloc(player, 9, 10);
    new_player->setPlayerNode(player_node);
    _world_node->addChild(player_node);
    _world->addObstacle(new_player);

    new_player->setDebugScene(_debug_node);
    new_player->setDebugColor(cugl::Color4(cugl::Color4::BLACK));
  }

  auto player = getPlayer(player_id);

  if (player) {
    cugl::Vec2 old_position = player->getPosition();

    // Movement must exceed this value to be animated
    if (abs(pos_x - old_position.x) > MIN_POS_CHANGE ||
        abs(pos_y - old_position.y) > MIN_POS_CHANGE) {
      player->setState(Player::MOVING);
    } else {
      player->setState(Player::IDLE);
    }
    // TODO does name need to be updated here? Yes if it seems to not be set
    // right?

    player->setRoomId(room_id);
    player->setNetworkPos(cugl::Vec2(pos_x, pos_y));
    player->updateDirection(pos_x - old_position.x, pos_y - old_position.y);
    player->animate();
  }
}

void PlayerController::interpolate(float timestep,
                                   const std::shared_ptr<Player>& player) {
  cugl::Vec2 new_pos = player->getNetworkPosCache()[0];
  cugl::Vec2 old_pos = player->getNetworkPosCache()[1];
  cugl::Vec2 cur_pos = old_pos;

  float time = player->getTimeSinceLastNetworkPosUpdate();

  if (time <= PLAYER_NETWORK_POS_UPDATE_MAX) {
    cur_pos.lerp(new_pos, time / PLAYER_NETWORK_POS_UPDATE_MAX);
  } else {
    cur_pos = new_pos;
  }

  player->setPosition(cur_pos);
}

void PlayerController::move(float timestep) {
  cugl::Vec2 forward = InputController::get<Movement>()->getMovement();
  bool is_dashing = InputController::get<Dash>()->isDashing();

  if (!_player->getDead()) {
    float speed = 175.0f;  // Movement speed.

    b2Filter filter_data = _player->getFilterData();
    filter_data.maskBits = MASK_PLAYER;

    if (is_dashing) {
      filter_data.maskBits = MASK_PLAYER_DASHING;
      forward = _player->getLastMoveDir();
      speed *= 10.0f;
    } else if (_player->getState() == Player::ATTACKING &&
               _player->_attack_frame_count <= ATTACK_STOP_MOVEMENT) {
      filter_data.maskBits = MASK_PLAYER_ATTACKING;
      forward = _player->getLastMoveDir();
      speed *= 1.2f;
    }

    _player->setFilterData(filter_data);
    _player->move(forward, speed);

    // Switch states.
    if (forward.x != 0 || forward.y != 0) {
      _player->setState(Player::MOVING);
      if (is_dashing) _player->setState(Player::State::DASHING);
    } else {
      _player->setState(Player::IDLE);
    }
  } else {
    _player->move(cugl::Vec2(0, 0), 0.0f);
    _player->setState(Player::IDLE);
  }
}

void PlayerController::attack() {
  bool did_attack = InputController::get<Attack>()->isAttacking();
  int time_held_down = InputController::get<Attack>()->timeHeldDown();

  if (!_player->getDead()) {
    if (time_held_down >= HOLD_ATTACK_COUNT) {
      _player->getPlayerNode()->setColor(cugl::Color4::BLUE);
      _player->_can_make_slash = true;
    } else {
      if (did_attack) {
        if (_player->_hurt_frames <= 0) {
          _player->getPlayerNode()->setColor(cugl::Color4::WHITE);
        }
        if (_player->_attack_frame_count == ATTACK_FRAMES) {
          _player->_frame_count = 0;

          cugl::Vec2 attackDir;
          switch (_player->getMoveDir()) {
            case 0:  // LEFT
              attackDir.set(-1, 0);
              break;
            case 1:  // DOWN
              attackDir.set(0, -1);
              break;
            case 2:  // RIGHT
              attackDir.set(1, 0);
              break;
            case 3:  // UP
              attackDir.set(0, 1);
              break;
          }

          if (_player->_can_make_slash) {
            _player->makeSlash(attackDir, _sword->getPosition());
            _player->_can_make_slash = false;
            _player->getPlayerNode()->setColor(cugl::Color4::WHITE);
          }
        }
        _sword->setEnabled(true);
        _player->setState(Player::ATTACKING);
        _player->_attack_frame_count--;
      }

      // If attacking, decrement the attack frames.
      if (_player->_attack_frame_count < ATTACK_FRAMES) {
        _player->setState(Player::ATTACKING);
        _player->_attack_frame_count--;
      }

      // If done attacking, switch to Idle.
      if (_player->_attack_frame_count <= 0) {
        _player->setState(Player::IDLE);
        _player->_frame_count = 0;
        _sword->setEnabled(false);
        _player->_attack_frame_count = ATTACK_FRAMES;
      }
    }

    // Set the _sword adjacent to the player
    _sword->moveSword(_player->getPosition() + _player->getOffset(),
                      cugl::Vec2(_player->getVX(), _player->getVY()),
                      _player->getMoveDir());
  }
}

void PlayerController::updateSlashes(float timestep) {
  auto proj = _player->getSlashes();
  auto it = proj.begin();
  while (it != proj.end()) {
    // Add to world if needed
    if ((*it)->getNode() == nullptr) {
      _world->addObstacle((*it));
      auto proj_node = cugl::scene2::SpriteNode::alloc(_slash_texture, 1, 7);
      proj_node->setPosition((*it)->getPosition());
      proj_node->flipHorizontal(_player->getMoveDir() == 0);
      if (_player->getMoveDir() == 1) {
        proj_node->setAngle(-M_PI / 2);
      } else if (_player->getMoveDir() == 3) {
        proj_node->setAngle(M_PI / 2);
      }
      (*it)->setNode(proj_node);
      _world_node->addChild(proj_node);
      (*it)->setDebugScene(_debug_node);
      (*it)->setDebugColor(cugl::Color4f::BLACK);
      (*it)->setInWorld(true);
    }

    (*it)->decrementFrame(1);
    (*it)->getNode()->setPosition((*it)->getPosition());
    if ((*it)->getFrames() % SLASH_FRAMES == 0) {
      (*it)->getNode()->setFrame((*it)->getNode()->getFrame() + 1);
    }
    ++it;
  }
}
