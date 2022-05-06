#include "PlayerController.h"

#include "../network/NetworkController.h"
#include "../network/structs/PlayerStructs.h"
#include "CollisionFiltering.h"
#include "actions/Attack.h"
#include "actions/Corrupt.h"
#include "actions/Dash.h"
#include "actions/Movement.h"
#include "actions/TargetPlayer.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 100

#define HOLD_ATTACK_COLOR_LIMIT 5
#define HOLD_ATTACK_COUNT 1000 /* milliseconds */
#define ATTACK_FRAMES 25
#define ATTACK_STOP_MOVEMENT 10
// Sync with Player.cpp
#define HURT_FRAMES 20
#define DEAD_FRAMES 175
// MAX_LIVE_FRAMES in projectile.cpp MUST be SLASH_FRAMES * 6
#define SLASH_FRAMES 7

#define HEALTH 100

#define MIN_POS_CHANGE 0.5f

// The max number of milliseconds between player network position updates.
#define PLAYER_NETWORK_POS_UPDATE_MAX 100.0f

#pragma mark PlayerController

PlayerController::PlayerController(){};

bool PlayerController::init(
    const std::shared_ptr<cugl::AssetManager>& assets,
    const std::shared_ptr<cugl::physics2::ObstacleWorld>& world,
    const std::shared_ptr<cugl::scene2::SceneNode>& world_node,
    const std::shared_ptr<cugl::scene2::SceneNode>& debug_node) {
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
      [=](const Sint32& code,
          const cugl::CustomNetworkDeserializer::CustomMessage& msg) {
        this->processData(code, msg);
      });

  return true;
}

void PlayerController::update(float timestep) {
  for (auto it : _players) {
    if (it.first != _player->getPlayerId()) interpolate(timestep, it.second);
  }

  if (_player->isBetrayer() && _player->canCorrupt() &&
      InputController::get<Corrupt>()->holdCorrupt()) {
    _player->move(cugl::Vec2(0, 0), 0.0f);
  } else {
    move(timestep);
    attack();
  }
  updateSlashes(timestep);

  for (auto it : _players) {
    _trail_managers[it.first]->run(it.second->getState() ==
                                   Player::State::DASHING);
    _trail_managers[it.first]->update();

    if (it.second->getState() == Player::State::MOVING &&
        it.second->isSteppingOnFloor()) {
      _sound_controller->playPlayerFootstep(
          SoundController::FootstepType::GRASS);
    }

    if (it.second->isHit()) _sound_controller->playPlayerHit();
  }

  if (InputController::get<Attack>()->chargeStart()) {
    _sound_controller->playPlayerEnergyCharge();
  } else if (InputController::get<Attack>()->attackReleased()) {
    _sound_controller->stopPlayerEnergyCharge();
  }

  if (_player->_hurt_frames == 0) {
    _player->getPlayerNode()->setColor(cugl::Color4::WHITE);
  }
  _player->_hurt_frames--;

  if (_player->_corrupt_count == 0) {
    _player->getPlayerNode()->setColor(cugl::Color4::WHITE);
  }
  _player->_corrupt_count--;

  if (_player->_blocked_corrupt_count == 0) {
    _player->setCanCorrupt(true);
    InputController::get<Corrupt>()->setActive(true);
  }
  _player->_blocked_corrupt_count--;

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

void PlayerController::blockCorrupt() {
  if (_player->isBetrayer()) {
    _player->setCanCorrupt(false);
    InputController::get<Corrupt>()->setActive(false);
  }
}

std::shared_ptr<Player> PlayerController::makePlayer(int player_id) {
  std::shared_ptr<cugl::Texture> player = _assets->get<cugl::Texture>("player");

  auto new_player = Player::alloc(cugl::Vec2::ZERO, "Johnathan");
  new_player->setPlayerId(player_id);

  auto player_node = cugl::scene2::SpriteNode::alloc(player, 9, 10);
  new_player->setPlayerNode(player_node);
  _world_node->addChild(player_node);
  _world->addObstacle(new_player);

  new_player->setDebugScene(_debug_node);
  new_player->setDebugColor(cugl::Color4(cugl::Color4::BLACK));
  new_player->setBasicInfoSentToHost(NetworkController::get()->isHost());

  addPlayer(new_player);
  addTrailManager(new_player);

  return new_player;
}

void PlayerController::processData(
    const Sint32& code,
    const cugl::CustomNetworkDeserializer::CustomMessage& msg) {
  switch (code) {
    case NC_HOST_ALL_PLAYER_INFO: {
      auto all_info =
          std::get<std::vector<std::shared_ptr<cugl::Serializable>>>(msg);

      for (std::shared_ptr<cugl::Serializable> info_ : all_info) {
        auto info = std::dynamic_pointer_cast<cugl::PlayerInfo>(info_);
        processPlayerInfo(info->player_id, info->room_id, info->pos);
      }
    } break;
    case NC_HOST_ALL_PLAYER_OTHER_INFO: {
      auto all_info =
          std::get<std::vector<std::shared_ptr<cugl::Serializable>>>(msg);

      for (std::shared_ptr<cugl::Serializable> info_ : all_info) {
        auto info = std::dynamic_pointer_cast<cugl::PlayerOtherInfo>(info_);
        processPlayerOtherInfo(info->player_id, info->energy, info->corruption);
      }
    } break;
    case NC_HOST_ALL_PLAYER_BASIC_INFO: {
      auto all_info =
          std::get<std::vector<std::shared_ptr<cugl::Serializable>>>(msg);

      for (std::shared_ptr<cugl::Serializable> info_ : all_info) {
        auto info = std::dynamic_pointer_cast<cugl::BasicPlayerInfo>(info_);
        processBasicPlayerInfo(info->player_id, info->name, info->betrayer);
      }
    } break;

    case NC_CLIENT_ONE_PLAYER_INFO: {
      auto info = std::dynamic_pointer_cast<cugl::PlayerInfo>(
          std::get<std::shared_ptr<cugl::Serializable>>(msg));
      processPlayerInfo(info->player_id, info->room_id, info->pos);
    } break;
    case NC_CLIENT_PLAYER_OTHER_INFO: {
      auto info = std::dynamic_pointer_cast<cugl::PlayerOtherInfo>(
          std::get<std::shared_ptr<cugl::Serializable>>(msg));
      processPlayerOtherInfo(info->player_id, info->energy, info->corruption);
    } break;
    case NC_CLIENT_PLAYER_BASIC_INFO: {
      auto info = std::dynamic_pointer_cast<cugl::BasicPlayerInfo>(
          std::get<std::shared_ptr<cugl::Serializable>>(msg));
      processBasicPlayerInfo(info->player_id, info->name, info->betrayer);
    } break;

    case NC_TERMINAL_ENERGY_UPDATE: {
      auto info = std::dynamic_pointer_cast<cugl::TerminalUpdate>(
          std::get<std::shared_ptr<cugl::Serializable>>(msg));
      if (info->player_id == _player->getPlayerId()) {
        _player->setEnergy(info->player_energy);
        _player->setCorruptedEnergy(info->player_corrupted_energy);
      }
    } break;
    default:
      break;
  }
}

void PlayerController::processPlayerInfo(int player_id, int room_id,
                                         cugl::Vec2& pos) {
  if (player_id == _player->getPlayerId()) return;

  auto player = getPlayerOrMakePlayer(player_id);

  cugl::Vec2 old_pos = player->getNetworkPosCache()[0];
  cugl::Vec2 diff = pos - old_pos;

  // Movement must exceed this value to be animated
  if (abs(diff.x) > MIN_POS_CHANGE || abs(diff.y) > MIN_POS_CHANGE) {
    player->setState(Player::MOVING);
  } else {
    player->setState(Player::IDLE);
    pos = old_pos;
  }

  player->setNetworkPos(pos);
  player->setRoomId(room_id);
  player->updateDirection(diff);
  player->animate();
}

void PlayerController::processPlayerOtherInfo(int player_id, int energy,
                                              int corruption) {
  auto player = getPlayerOrMakePlayer(player_id);

  player->setEnergy(energy);
  player->setCorruptedEnergy(corruption);
}

void PlayerController::processBasicPlayerInfo(int player_id,
                                              const std::string& display_name,
                                              bool is_betrayer) {
  if (player_id == _player->getPlayerId()) return;

  auto player = getPlayerOrMakePlayer(player_id);

  auto pixelmix_font = _assets->get<cugl::Font>("pixelmix_extra_extra_small");
  // Display different color if curr player and other player both betrayers.
  bool display_betrayer = is_betrayer && _player->isBetrayer();
  player->setBetrayer(is_betrayer);
  player->setDisplayName(display_name);
  player->setNameNode(pixelmix_font, display_betrayer);

  auto energy_fill = _assets->get<cugl::Texture>("energy-fill-small");
  auto energy_bar = _assets->get<cugl::Texture>("energy-bar-small");
  auto energy_outline = _assets->get<cugl::Texture>("energy-outline-small");

  auto regular_bar = cugl::scene2::ProgressBar::alloc(energy_fill, energy_bar);
  regular_bar->addChild(
      cugl::scene2::PolygonNode::allocWithTexture(energy_outline));
  regular_bar->setForegroundColor(cugl::Color4("#9ec1de"));
  // Orange for other betrayers.
  if (is_betrayer) regular_bar->setForegroundColor(cugl::Color4("#df7126"));

  auto corrupted_bar =
      cugl::scene2::ProgressBar::alloc(energy_fill, energy_bar);
  corrupted_bar->setForegroundColor(cugl::Color4("#df7126"));

  if (_player->isBetrayer()) {
    // It's important that these be placed in this specific order.
    player->setCorruptedEnergyBar(corrupted_bar);
    player->setEnergyBar(regular_bar);
  }

  player->setBasicInfoSentToHost(true);
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
  if (!_player->getDead()) {
    if (InputController::get<Attack>()->chargeOver()) {
      _player->_can_make_slash = true;
      _player->getPlayerNode()->setColor(cugl::Color4(180, 180, 255));
    } else {
      if (InputController::get<Attack>()->isAttacking()) {
        if (_player->_hurt_frames <= 0) {
          _player->getPlayerNode()->setColor(cugl::Color4::WHITE);
        }
        if (_player->_attack_frame_count == ATTACK_FRAMES) {
          _player->_frame_count = 0;
          // Play player swing sound effect.
          _sound_controller->playPlayerSwing();

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
            _sound_controller->playPlayerEnergyWave();
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

void PlayerController::addTrailManager(const std::shared_ptr<Player>& player) {
  TrailManager::Config config;
  config.max_length = 5;
  config.freq = 4;
  config.max_opacity = 220;
  config.min_opacity = 140;
  config.color = cugl::Color4(200, 200, 255, 255);
  _trail_managers[player->getPlayerId()] =
      TrailManager::alloc(player->getPlayerNode(), config);
}
