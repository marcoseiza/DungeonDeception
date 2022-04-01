#include "Player.h"

#define IDLE_RIGHT 0
#define IDLE_LEFT 1
#define IDLE_DOWN 2
#define IDLE_UP 3
#define ATTACK_LOW_LIM 10
#define ATTACK_HIGH_LIM 16
#define RUN_LOW_LIM 20
#define RUN_HIGH_LIM 29
#define ATTACK_FRAMES 14
#define HEALTH 100

#define WIDTH 24.0f
#define HEIGHT 48.0f

#define HEIGHT_SHRINK 0.3f

#define HURT_FRAMES 10
#define DEAD_FRAMES 175

#pragma mark Init

bool Player::init(const cugl::Vec2 pos, string name) {
  cugl::Vec2 pos_ = pos;
  cugl::Size size_ = cugl::Size(WIDTH, HEIGHT);

  size_.height *= HEIGHT_SHRINK;

  _offset_from_center.y = HEIGHT / 2.0f - size_.height / 2.0f;
  pos_ -= _offset_from_center;

  CapsuleObstacle::init(pos_, size_);
  setName(name);

  _player_node = nullptr;
  _current_state = IDLE;
  _health = HEALTH;
  _frame_count = 0;
  _attack_frame_count = ATTACK_FRAMES;
  _hurt_frames = 0;
  _isDead = false;

  setDensity(0.01f);
  setFriction(0.0f);
  setRestitution(0.01f);
  setFixedRotation(true);

  return true;
}

void Player::takeDamage() {
  if (_hurt_frames <= 0) {
    reduceHealth(5);
    _player_node->setColor(cugl::Color4::RED);
    _hurt_frames = HURT_FRAMES;
  }
}

void Player::dies() {
  _isDead = true;
  _player_node->setColor(cugl::Color4::RED);
  _hurt_frames = DEAD_FRAMES;
}

#pragma mark Animation & Drawing

void Player::update(float delta) {
  CapsuleObstacle::update(delta);
  if (_player_node != nullptr) {
    if (_promise) {
      setPosition(_promise_pos_cache);
      _promise = false;
    }
    _player_node->setPosition(getPosition() + _offset_from_center);
  }
}

void Player::animate(float forwardX, float forwardY) {
  switch (_current_state) {
    case MOVING: {
      // Reverse texture if moving opposite direction.
      bool movingLeft = forwardX < 0;
      if (_player_node->isFlipHorizontal() != movingLeft) {
        _player_node->flipHorizontal(movingLeft);
      }

      if (_frame_count == 0) {
        _player_node->setFrame(RUN_LOW_LIM);
      }

      // Play the next animation frame.
      if (_frame_count >= 5) {
        _frame_count = 0;

        if (_player_node->isFlipHorizontal()) {
          if (_player_node->getFrame() <= RUN_LOW_LIM) {
            _player_node->setFrame(RUN_HIGH_LIM);
          } else {
            _player_node->setFrame(_player_node->getFrame() - 1);
          }
        } else {
          if (_player_node->getFrame() >= RUN_HIGH_LIM) {
            _player_node->setFrame(RUN_LOW_LIM);
          } else {
            _player_node->setFrame(_player_node->getFrame() + 1);
          }
        }
      }
      _frame_count++;
      break;
    }
    case IDLE: {
      if (_player_node->isFlipHorizontal()) {
        _player_node->setFrame(10 - IDLE_LEFT);
      } else {
        _player_node->setFrame(IDLE_RIGHT);
      }
      _frame_count = 0;
      break;
    }
    case ATTACKING: {
      if (_frame_count == 0) {
        if (_player_node->isFlipHorizontal()) {
          _player_node->setFrame(9 + ATTACK_LOW_LIM);
        } else {
          _player_node->setFrame(ATTACK_LOW_LIM);
        }
      }

      // Play the next animation frame.
      if (_frame_count >= 2) {
        _frame_count = 0;

        if (_player_node->isFlipHorizontal()) {
          if (_player_node->getFrame() <=
              19 - ATTACK_HIGH_LIM + ATTACK_LOW_LIM) {
            _player_node->setFrame(9 + ATTACK_LOW_LIM);
          } else {
            _player_node->setFrame(_player_node->getFrame() - 1);
          }
        } else {
          if (_player_node->getFrame() >= ATTACK_HIGH_LIM) {
            _player_node->setFrame(ATTACK_LOW_LIM);
          } else {
            _player_node->setFrame(_player_node->getFrame() + 1);
          }
        }
      }
      _frame_count++;
      break;
    }
  }
}

void Player::move(float forwardX, float forwardY) {
  setVX(200 * forwardX);
  setVY(200 * forwardY);
  if (forwardX == 0) setVX(0);
  if (forwardY == 0) setVY(0);
}

void Player::makeSlash(cugl::Vec2 attackDir, cugl::Vec2 swordPos) {
  // Make the sword slash projectile
  auto slash = Projectile::alloc(swordPos, attackDir);
  _slashes.emplace(slash);
  slash->setPosition(swordPos);

  slash->setName("slash");
}

void Player::checkDeleteSlashes(
    std::shared_ptr<cugl::physics2::ObstacleWorld> world,
    std::shared_ptr<cugl::scene2::SceneNode> world_node) {
  auto itt = _slashes.begin();
  while (itt != _slashes.end()) {
    if ((*itt)->getFrames() <= 0) {
      (*itt)->deactivatePhysics(*world->getWorld());
      world_node->removeChild((*itt)->getNode());
      world->removeObstacle((*itt).get());
      itt = _slashes.erase(itt);
    } else {
      ++itt;
    }
  }
}
