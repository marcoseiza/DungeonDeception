#include "Player.h"

#include "../controllers/CollisionFiltering.h"

#define IDLE_RIGHT 82
#define IDLE_LEFT 80
#define IDLE_DOWN 81
#define IDLE_UP 83
#define RUN_LIM_GAP 9
#define ATTACK_LIM_GAP 8
#define ATTACK_SETUP_LIM_GAP 7
#define DEATH_START_FRAME 90
#define FINAL_DEATH_FRAME 114
#define ATTACK_FRAMES 25
#define HEALTH 50

#define WIDTH 24.0f
#define HEIGHT 48.0f

#define HEIGHT_SHRINK 0.3f

#define HURT_FRAMES 20
#define DEAD_FRAMES 175

#define MIN_DIFF_FOR_DIR_CHANGE 0.5f

#define ENERGY_BAR_UPDATE_SIZE 0.02f

#define ENERGY_SLASH_SPEED 400
#define ENERGY_SLASH_LIFE 60

#define FLASH_BLOCK_ICON_LENGTH 30

#pragma mark Init

bool Player::init(const cugl::Vec2 pos, const std::string& name) {
  cugl::Vec2 pos_ = pos;
  cugl::Size size_ = cugl::Size(WIDTH, HEIGHT);

  size_.height *= HEIGHT_SHRINK;

  _offset_from_center.y = HEIGHT / 2.0f - size_.height / 2.0f;
  pos_ -= _offset_from_center;

  _network_pos_cache[0] = pos_;
  _network_pos_cache[1] = pos_;

  CapsuleObstacle::init(pos_, size_);
  setName(name);

  _player_node = nullptr;
  _current_state = IDLE;
  _health = HEALTH;
  _energy = 40;
  _corrupted_energy = 0;
  _frame_count = 0;
  _attack_frame_count = ATTACK_FRAMES;
  _hurt_frames = 0;
  _corrupt_count = 0;
  _isDead = false;
  _is_respawning = false;
  _mv_direc = IDLE_LEFT;
  _room_id = -1;
  _flash_block_icon_counter = -1;

  setDensity(0.01f);
  setFriction(0.0f);
  setRestitution(0.01f);
  setFixedRotation(true);

  _projectile_sensor = nullptr;
  _projectile_sensor_name = nullptr;

  _fixture.filter.categoryBits = CATEGORY_PLAYER;
  _fixture.filter.maskBits = MASK_PLAYER;

  _projectile_sensor_def.filter.categoryBits = CATEGORY_PLAYER;
  _projectile_sensor_def.filter.maskBits = MASK_PLAYER_PROJECTILE;

  return true;
}

void Player::dispose() {
  _player_node->setVisible(false);
  _player_node = nullptr;
}

void Player::setPlayerNode(
    const std::shared_ptr<cugl::scene2::SpriteNode>& node) {
  _player_node = node;
}

void Player::setNameNode(const std::shared_ptr<cugl::Font>& name_font,
                         bool display_betrayer) {
  if (_name_node == nullptr) {
    _name_node = cugl::scene2::TextField::allocWithText("", name_font);
    _name_node->setAnchor(.5, 0);
    _name_node->setName("player_name");
    _player_node->addChild(_name_node);
  }

  _name_node->setForeground(cugl::Color4::WHITE);
  if (display_betrayer) {
    _name_node->setForeground(cugl::Color4::RED);
    _name_node->setDropShadow(.75, -.75);
  }
  _name_node->setText(_display_name, true);

  cugl::Vec2 pos = _player_node->getContentSize() / 2.0f;
  pos.y *= 1.33;
  _name_node->setPosition(pos);
  _name_node->setPriority(std::numeric_limits<float>::max());
}

void Player::setEnergyBar(
    const std::shared_ptr<cugl::scene2::ProgressBar>& bar) {
  _energy_bar = bar;
  _player_node->addChild(_energy_bar);
  _energy_bar->setAnchor(0.5f, 0.5f);

  cugl::Vec2 pos = _player_node->getContentSize() / 2.0f;
  pos.y *= 1.38f;
  _energy_bar->setPosition(pos);
  _energy_bar->setPriority(std::numeric_limits<float>::max());

  // Push the name up to make room.
  cugl::Vec2 name_pos = _player_node->getContentSize() / 2.0f;
  name_pos.y *= 1.48;
  _name_node->setPosition(name_pos);

  for (auto child : _energy_bar->getChildren()) {
    child->setPriority(std::numeric_limits<float>::max());
  }
}

void Player::setCorruptedEnergyBar(
    const std::shared_ptr<cugl::scene2::ProgressBar>& bar) {
  _corrupted_energy_bar = bar;
  _player_node->addChild(_corrupted_energy_bar);
  _corrupted_energy_bar->setAnchor(0.5f, 0.5f);

  cugl::Vec2 pos = _player_node->getContentSize() / 2.0f;
  pos.y *= 1.38f;
  _corrupted_energy_bar->setPosition(pos);
  _corrupted_energy_bar->setPriority(std::numeric_limits<float>::max());

  for (auto child : _corrupted_energy_bar->getChildren()) {
    child->setPriority(std::numeric_limits<float>::max());
  }
}

void Player::setBlockIcon(const std::shared_ptr<cugl::scene2::SceneNode>& icon,
                          bool center) {
  _block_icon = icon;
  _player_node->addChild(_block_icon);
  _block_icon->setAnchor(cugl::Vec2::ANCHOR_CENTER);

  cugl::Vec2 pos = _player_node->getContentSize() / 2.0f;
  if (center) {
    pos.y *= 1.55f;
  } else {
    pos.y *= 1.38f;
    pos.x += 39;
  }
  _block_icon->setPosition(pos);
  _block_icon->setPriority(std::numeric_limits<float>::max());
}

void Player::flashBlockIcon() {
  _block_icon->setColor(cugl::Color4::RED);
  _block_icon->setScale(_block_icon->getScale() * 1.2f);
  _flash_block_icon_counter = FLASH_BLOCK_ICON_LENGTH;
}

void Player::takeDamage() {
  if (_hurt_frames <= 0) {
    reduceHealth(5);
    _player_node->setColor(cugl::Color4::RED);
    _hurt_frames = HURT_FRAMES;
  }
}

bool Player::isHit() const { return _hurt_frames == HURT_FRAMES; }

void Player::dies() {
  _isDead = true;
  _hurt_frames = DEAD_FRAMES;
  _player_node->setColor(cugl::Color4::WHITE);
  setSensor(true);
}

void Player::setCorrupted() {
  if (_is_betrayer) {
    _corrupt_count = 10;
    _player_node->setColor(cugl::Color4::ORANGE);
  }
}

void Player::toggleBlockPlayerOnBetrayer(int runner_id) {
  if (!_is_betrayer) return;
  if (_blocked_players.find(runner_id) == _blocked_players.end()) {
    _blocked_players.insert(runner_id);  // 1 minute
  } else {
    _blocked_players.erase(runner_id);
  }
}

#pragma mark Animation & Drawing

void Player::createFixtures() {
  if (_body == nullptr) return;

  CapsuleObstacle::createFixtures();

  if (_projectile_sensor == nullptr) {
    _projectile_sensor_def.density = 0.0f;
    _projectile_sensor_def.isSensor = true;
    _projectile_sensor_name =
        std::make_shared<std::string>("player_projectile_sensor");
    _projectile_sensor_def.userData.pointer =
        reinterpret_cast<uintptr_t>(_projectile_sensor_name.get());

    // Dimensions
    b2Vec2 corners[4];
    corners[0].x = -CapsuleObstacle::getWidth() / 1.5f;
    corners[0].y = CapsuleObstacle::getHeight() / 1.0f;
    corners[1].x = -CapsuleObstacle::getWidth() / 1.5f;
    corners[1].y = -CapsuleObstacle::getHeight() / 1.5f;
    corners[2].x = CapsuleObstacle::getWidth() / 1.5f;
    corners[2].y = -CapsuleObstacle::getHeight() / 1.5f;
    corners[3].x = CapsuleObstacle::getWidth() / 1.5f;
    corners[3].y = CapsuleObstacle::getHeight() / 1.0f;

    b2PolygonShape sensorShape;
    sensorShape.Set(corners, 4);

    _projectile_sensor_def.shape = &sensorShape;
    _projectile_sensor = _body->CreateFixture(&_projectile_sensor_def);
  }
}

void Player::releaseFixtures() {
  if (_body == nullptr) return;

  CapsuleObstacle::releaseFixtures();

  if (_projectile_sensor != nullptr) {
    _body->DestroyFixture(_projectile_sensor);
    _projectile_sensor = nullptr;
  }
}

void Player::update(float delta) {
  CapsuleObstacle::update(delta);
  if (_player_node != nullptr) {
    if (_promise_pos_cache) {
      setPosition(*_promise_pos_cache);
      _promise_pos_cache = std::nullopt;
    }
    _player_node->setPosition(getPosition() + _offset_from_center);
  }

  // Animate the energy bars.
  if (_energy_bar != nullptr && _corrupted_energy_bar != nullptr) {
    float target_energy = (_energy - _corrupted_energy) / 100.0f;
    float target_corrupt_energy = _energy / 100.0f;

    if (_energy_bar->getProgress() < target_energy) {
      float df = _energy_bar->getProgress() + ENERGY_BAR_UPDATE_SIZE;
      _energy_bar->setProgress(std::min(df, target_energy));
    } else if (_energy_bar->getProgress() > target_energy) {
      float df = _energy_bar->getProgress() - ENERGY_BAR_UPDATE_SIZE;
      _energy_bar->setProgress(std::max(df, target_energy));
    }

    if (_corrupted_energy_bar->getProgress() < target_corrupt_energy) {
      float df = _corrupted_energy_bar->getProgress() + ENERGY_BAR_UPDATE_SIZE;
      _corrupted_energy_bar->setProgress(std::min(df, target_corrupt_energy));
    } else if (_corrupted_energy_bar->getProgress() > target_corrupt_energy) {
      float df = _corrupted_energy_bar->getProgress() - ENERGY_BAR_UPDATE_SIZE;
      _corrupted_energy_bar->setProgress(std::max(df, target_corrupt_energy));
    }
  }

  if (_flash_block_icon_counter > 0) {
    _flash_block_icon_counter--;
  } else if (_flash_block_icon_counter == 0) {
    if (_block_icon) {
      _flash_block_icon_counter = -1;
      _block_icon->setColor(cugl::Color4::WHITE);
      _block_icon->setScale(_block_icon->getScale() * 5.f / 6.f);
    }
  }
}

void Player::animate() {
  if (!_isDead) {
    switch (_current_state) {
      case DASHING:
      case MOVING: {
        if (_frame_count == 0) {
          _player_node->setFrame(getRunLowLim());
        }

        // Play the next animation frame.
        if (_frame_count >= 5) {
          _frame_count = 0;
          if (_player_node->getFrame() >= getRunHighLim()) {
            _player_node->setFrame(getRunLowLim());
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
  } else {
    if (_mv_direc == IDLE_LEFT) {
      _player_node->flipHorizontal(true);
    }
    // Death just began, set to initial frame.
    if (_player_node->getFrame() < DEATH_START_FRAME) {
      _player_node->setFrame(DEATH_START_FRAME);
    }
    
    if (_player_node->getFrame() != FINAL_DEATH_FRAME) {
      if (_frame_count >= 5) {
        _frame_count = 0;
        _player_node->setFrame(_player_node->getFrame() + 1);
      }
      _frame_count++;
    }
  }
}

void Player::move(const cugl::Vec2& forward, float speed) {
  _last_move_dir.set(forward.x, forward.y);

  setVX(speed * forward.x);
  setVY(speed * forward.y);
  if (forward.x == 0) setVX(0);
  if (forward.y == 0) setVY(0);

  // Set the correct frame for idle
  updateDirection(forward);
}

void Player::updateDirection(const cugl::Vec2& diff) {
  int new_direc = _mv_direc;
  bool sufficiently_equal = (std::abs(std::abs(diff.x) - std::abs(diff.y)) <=
                             MIN_DIFF_FOR_DIR_CHANGE);

  if (std::abs(diff.x) >= std::abs(diff.y) || sufficiently_equal) {
    if (diff.x < 0) {
      new_direc = IDLE_LEFT;
    } else if (diff.x > 0) {
      new_direc = IDLE_RIGHT;
    }
  } else if (std::abs(diff.x) < std::abs(diff.y)) {
    if (diff.y < 0) {
      new_direc = IDLE_DOWN;
    } else if (diff.y > 0) {
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
  auto slash = Projectile::alloc(swordPos, attackDir, ENERGY_SLASH_SPEED,
                                 ENERGY_SLASH_LIFE);
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

int Player::getRunLowLim() { return getRunHighLim() - RUN_LIM_GAP; }

bool Player::isSteppingOnFloor() {
  int relative_frame = _player_node->getFrame() - getRunLowLim();
  return (relative_frame == 1 || relative_frame == 6) && _frame_count == 1;
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
