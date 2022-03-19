#include "PlayerController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 100

#define ATTACK_FRAMES 14
#define HURT_FRAMES 10
#define DEAD_FRAMES 175

#define HEALTH 100

#pragma mark PlayerController

PlayerController::PlayerController(){};

bool PlayerController::init(
    std::shared_ptr<Player> player, std::shared_ptr<cugl::AssetManager> assets,
    std::shared_ptr<cugl::physics2::ObstacleWorld> world,
    std::shared_ptr<cugl::scene2::SceneNode> world_node,
    std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
  _player = player;
  _slash_texture = assets->get<cugl::Texture>("energy-slash");
  _world = world;
  _world_node = world_node;
  _debug_node = debug_node;

  return true;
}

void PlayerController::update(float timestep, cugl::Vec2 forward,
                              bool didAttack, std::shared_ptr<Sword> sword) {
  move(timestep, forward);
  attack(didAttack, sword);
  updateSlashes(timestep);

  if (_player->_hurt_frames <= 0) {
    _player->getPlayerNode()->setColor(cugl::Color4::WHITE);
    _player->_hurt_frames = 0;
  } else {
    _player->_hurt_frames--;
  }

  // CHECK IF RAN OUT OF HEALTH
  if (_player->getHealth() <= 0 && !_player->getDead()) {
    _player->dies();
  }

  // CHECK IF HAS BEEN DEAD FOR LONG ENOUGH TO REVIVE
  if (_player->getDead() && _player->_hurt_frames == 0) {
    _player->setHealth(HEALTH);
    _player->setDead(false);
  }

  // Animate the player
  _player->animate(forward);
}

void PlayerController::move(float timestep, cugl::Vec2 forward) {
  if (!_player->getDead()) {
    _player->move(forward);

    // Switch states.
    if (forward.x != 0 || forward.y != 0) {
      _player->setState(Player::MOVING);
    } else {
      _player->setState(Player::IDLE);
    }
  } else {
    _player->move(cugl::Vec2(0, 0));
    _player->setState(Player::IDLE);
  }
}

void PlayerController::attack(bool didAttack, std::shared_ptr<Sword> sword) {
  if (!_player->getDead()) {
    if (didAttack) {
      // There is not a slash direction.
      if (_player->_attack_frame_count == ATTACK_FRAMES) {
        _player->_frame_count = 0;
        cugl::Vec2 attackDir = cugl::Vec2(1, 0);
        if (_player->getPlayerNode()->isFlipHorizontal()) {
          attackDir = cugl::Vec2(-1, 0);
        }
        _player->makeSlash(attackDir, sword->getPosition());
      }
      sword->setEnabled(true);
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
      sword->setEnabled(false);
      _player->_attack_frame_count = ATTACK_FRAMES;
    }

    // Set the sword adjacent to the player
    sword->moveSword(_player->getPosition() + _player->getOffset(),
                     cugl::Vec2(_player->getVX(), _player->getVY()),
                     _player->getPlayerNode()->isFlipHorizontal());
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
      proj_node->flipHorizontal(_player->getPlayerNode()->isFlipHorizontal());
      (*it)->setNode(proj_node);
      _world_node->addChild(proj_node);
      (*it)->setDebugScene(_debug_node);
      (*it)->setDebugColor(cugl::Color4f::BLACK);
      (*it)->setInWorld(true);
    }

    (*it)->decrementFrame(1);
    (*it)->getNode()->setPosition((*it)->getPosition());
    ++it;
  }
}
