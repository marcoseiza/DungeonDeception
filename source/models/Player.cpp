#include "Player.h"

#include "../controllers/CollisionFiltering.h"

#define IDLE_RIGHT 82
#define IDLE_LEFT 80
#define IDLE_DOWN 81
#define IDLE_UP 83
#define RUN_LIM_GAP 9
#define ATTACK_LIM_GAP 8
#define ATTACK_FRAMES 25
#define HEALTH 100

#define WIDTH 24.0f
#define HEIGHT 48.0f

#define HEIGHT_SHRINK 0.3f

#define HURT_FRAMES 20
#define DEAD_FRAMES 175

#define MIN_DIFF_FOR_DIR_CHANGE 0.05

#pragma mark Init

bool Player::init(const cugl::Vec2 pos, string name, string display_name) {
  cugl::Vec2 pos_ = pos;
  cugl::Size size_ = cugl::Size(WIDTH, HEIGHT);

  size_.height *= HEIGHT_SHRINK;

  _offset_from_center.y = HEIGHT / 2.0f - size_.height / 2.0f;
  pos_ -= _offset_from_center;

  _network_pos_cache[0] = pos_;
  _network_pos_cache[1] = pos_;

  CapsuleObstacle::init(pos_, size_);
  setName(name);

  _display_name = display_name;

  _player_node = nullptr;
  _current_state = IDLE;
  _health = HEALTH;
  _luminance = 40;
  _corrupted_luminance = 0;
  _frame_count = 0;
  _attack_frame_count = ATTACK_FRAMES;
  _hurt_frames = 0;
  _corrupt_count = 0;
  _isDead = false;
  _is_respawning = false;
  _mv_direc = IDLE_LEFT;
  _room_id = -1;

  setDensity(0.01f);
  setFriction(0.0f);
  setRestitution(0.01f);
  setFixedRotation(true);

  _fixture.filter.categoryBits = CATEGORY_PLAYER;
  _fixture.filter.maskBits = MASK_PLAYER;

  return true;
}

void Player::setPlayerNode(
    const std::shared_ptr<cugl::scene2::SpriteNode>& node) {
  _player_node = node;
}

void Player::setNameNode(std::shared_ptr<cugl::Font> name_font,
                         bool display_betrayer) {
  _name_node = cugl::scene2::TextField::allocWithText(_display_name, name_font);
  _name_node->setForeground(cugl::Color4::WHITE);
  if (display_betrayer) {
    _name_node->setForeground(cugl::Color4::RED);
    _name_node->setDropShadow(.75, -.75);
  }
  _name_node->setAnchor(.5, 0);
  _name_node->setName("player_name");

  _player_node->addChild(_name_node);
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

void Player::setCorrupted() {
  if (_is_betrayer) {
    _corrupt_count = 10;
    _player_node->setColor(cugl::Color4::ORANGE);
  }
}

#pragma mark Animation & Drawing

void Player::update(float delta) {
  CapsuleObstacle::update(delta);
  if (_player_node != nullptr) {
    if (_promise_pos_cache) {
      setPosition(*_promise_pos_cache);
      _promise_pos_cache = std::nullopt;
    }
    _player_node->setPosition(getPosition() + _offset_from_center);

    if (_name_node != nullptr) {
      _name_node->setPosition(_player_node->getWidth() / 2.0f,
                              _player_node->getHeight() / 1.45f);
    }
  }
}

void Player::animate() {
  switch (_current_state) {
    case DASHING:
    case MOVING: {
      int run_high_lim = getRunHighLim();
      int run_low_lim = run_high_lim - RUN_LIM_GAP;

      if (_frame_count == 0) {
        _player_node->setFrame(run_low_lim);
      }

      // Play the next animation frame.
      if (_frame_count >= 5) {
        _frame_count = 0;
        if (_player_node->getFrame() >= run_high_lim) {
          _player_node->setFrame(run_low_lim);
        } else {
          _player_node->setFrame(_player_node->getFrame() + 1);
        }
      }
      _frame_count++;
      break;
    }
    case IDLE: {
      _player_node->setFrame(_mv_direc);
      _frame_count = 0;
      break;
    }
    case ATTACKING: {
      int attack_high_lim = getAttackHighLim();
      int attack_low_lim = attack_high_lim - ATTACK_LIM_GAP;

      // Play the next animation frame.
      if (_frame_count >= 3) {
        _frame_count = 0;
        if (_player_node->getFrame() >= attack_high_lim) {
          _player_node->setFrame(attack_low_lim);
        } else {
          _player_node->setFrame(_player_node->getFrame() + 1);
        }
      }
      _frame_count++;
      break;
    }
  }
}

void Player::move(float forwardX, float forwardY, float speed) {
  _last_move_dir.set(forwardX, forwardY);

  setVX(speed * forwardX);
  setVY(speed * forwardY);
  if (forwardX == 0) setVX(0);
  if (forwardY == 0) setVY(0);

  // Set the correct frame for idle
  updateDirection(forwardX, forwardY);
}

void Player::updateDirection(float x_diff, float y_diff) {
  int new_direc = _mv_direc;
  bool sufficiently_equal = (std::abs(std::abs(x_diff) - std::abs(y_diff)) <=
                             MIN_DIFF_FOR_DIR_CHANGE);

  if (std::abs(x_diff) >= std::abs(y_diff) || sufficiently_equal) {
    if (x_diff < 0) {
      new_direc = IDLE_LEFT;
    } else if (x_diff > 0) {
      new_direc = IDLE_RIGHT;
    }
  } else if (std::abs(x_diff) < std::abs(y_diff)) {
    if (y_diff < 0) {
      new_direc = IDLE_DOWN;
    } else if (y_diff > 0) {
      new_direc = IDLE_UP;
    }
  }
  if (new_direc != _mv_direc) {
    _mv_direc = new_direc;
    _frame_count = 0;
  }
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

int Player::getRunHighLim() {
  if (_mv_direc == IDLE_RIGHT) {
    return 59;  // Value for the right run high limit
  } else if (_mv_direc == IDLE_LEFT) {
    return 69;  // Value for the left run high limit
  } else if (_mv_direc == IDLE_UP) {
    return 79;  // Value for the up run high limit
  }
  return 49;  // Value for the down run high limit
}

int Player::getAttackHighLim() {
  if (_mv_direc == IDLE_RIGHT) {
    return 8;  // Value for the right attack high limit
  } else if (_mv_direc == IDLE_LEFT) {
    return 28;  // Value for the left attack high limit
  } else if (_mv_direc == IDLE_UP) {
    return 38;  // Value for the up attack high limit
  }
  return 18;  // Value for the down attack high limit
}
