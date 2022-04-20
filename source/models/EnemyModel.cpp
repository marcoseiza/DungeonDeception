#include "EnemyModel.h"

#include "../controllers/CollisionFiltering.h"

#define WIDTH 24.0f
#define HEIGHT 48.0f

#define HEIGHT_SHRINK 0.3f

#pragma mark Init
bool EnemyModel::init(const cugl::Vec2 pos, string name, string type) {
  cugl::Vec2 pos_ = pos;
  cugl::Size size_ = cugl::Size(WIDTH, HEIGHT);

  size_.height *= HEIGHT_SHRINK;
  _offset_from_center.y = HEIGHT / 2.0f - size_.height / 2.0f;
  pos_ -= _offset_from_center;

  CapsuleObstacle::init(pos_, size_);

  setName(name);
  setType(type);

  _enemy_node = nullptr;
  _health = 100;
  _facing_left = false;
  _atc_timer = 0;
  _cta_timer = 0;
  _isKnockbacked = false;
  _stunned_timer = 0;

  _attack_cooldown = 0;

  setDensity(0.5f);
  setFriction(0.5f);
  setRestitution(0.5f);
  setFixedRotation(true);

  _fixture.filter.categoryBits = CATEGORY_ENEMY;
  _fixture.filter.maskBits = MASK_ENEMY;

  _hitbox_sensor_def.filter.categoryBits = CATEGORY_ENEMY_HITBOX;
  _hitbox_sensor_def.filter.maskBits = MASK_ENEMY_HITBOX;

  _damage_sensor_def.filter.categoryBits = CATEGORY_ENEMY_DAMAGE;
  _damage_sensor_def.filter.maskBits = MASK_ENEMY_DAMAGE;

  if (_enemy_type == TURTLE) {
    setBodyType(b2BodyType::b2_staticBody);
  }

  // Initialize the polygon nodes
  for (int i = 0; i < 16; i++) {
    auto p = cugl::scene2::PolygonNode::alloc();
    _polys.emplace_back(p);
  }

  return true;
}

void EnemyModel::takeDamage(float amount) {
  reduceHealth(amount);
  _enemy_node->setColor(cugl::Color4::RED);
  _damage_count = 10;
}

void EnemyModel::addBullet(const cugl::Vec2 p) {
  cugl::Vec2 diff = p - getPosition();
  auto bullet = Projectile::alloc(
      cugl::Vec2(getPosition().x, getPosition().y + _offset_from_center.y),
      diff);

  _projectiles.emplace(bullet);
  bullet->setPosition(
      cugl::Vec2(getPosition().x, getPosition().y + _offset_from_center.y));
}

void EnemyModel::deleteProjectile(
    std::shared_ptr<cugl::physics2::ObstacleWorld> _world,
    std::shared_ptr<cugl::scene2::SceneNode> _world_node) {
  auto itt = _projectiles.begin();
  while (itt != _projectiles.end()) {
    if ((*itt)->getFrames() <= 0) {
      (*itt)->deactivatePhysics(*_world->getWorld());
      _world_node->removeChild((*itt)->getNode());
      _world->removeObstacle((*itt).get());
      itt = _projectiles.erase(itt);
    } else {
      ++itt;
    }
  }
}

void EnemyModel::deleteAllProjectiles(
    std::shared_ptr<cugl::physics2::ObstacleWorld> _world,
    std::shared_ptr<cugl::scene2::SceneNode> _world_node) {
  auto itt = _projectiles.begin();
  while (itt != _projectiles.end()) {
    (*itt)->deactivatePhysics(*_world->getWorld());
    _world_node->removeChild((*itt)->getNode());
    _world->removeObstacle((*itt).get());
    ++itt;
  }
  _projectiles.clear();
}

void EnemyModel::setType(std::string type) {
  if (type == "grunt") {
    _enemy_type = GRUNT;
    _speed = 0.7;
  } else if (type == "shotgunner") {
    _enemy_type = SHOTGUNNER;
    _speed = 0.6;
  } else if (type == "tank") {
    _enemy_type = TANK;
    _speed = 0.6;
  } else if (type == "turtle") {
    _enemy_type = TURTLE;
    _speed = 0;
  }
}

#pragma mark Animation & Drawing

void EnemyModel::setNode(const std::shared_ptr<cugl::scene2::SpriteNode>& node,
                         std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
  _enemy_node = node;

  // Add the ray cast weights to the debug node.
  for (std::shared_ptr<cugl::scene2::PolygonNode> poly : _polys) {
    debug_node->addChild(poly);
  }
}

std::shared_ptr<cugl::scene2::SpriteNode>& EnemyModel::getNode() {
  return _enemy_node;
}

#pragma mark -
#pragma mark Physics Methods

void EnemyModel::createFixtures() {
  if (_body == nullptr) return;

  CapsuleObstacle::createFixtures();

  if (_hitbox_sensor == nullptr) {
    _hitbox_sensor_def.density = 0.0f;
    _hitbox_sensor_def.isSensor = true;
    _hitbox_sensor_name = std::make_shared<std::string>("enemy_hitbox");
    _hitbox_sensor_def.userData.pointer =
        reinterpret_cast<uintptr_t>(_hitbox_sensor_name.get());

    // Sensor dimensions
    b2Vec2 corners[4];
    corners[0].x = -CapsuleObstacle::getWidth() / 2.0f;
    corners[0].y = HEIGHT;
    corners[1].x = -CapsuleObstacle::getWidth() / 2.0f;
    corners[1].y = -HEIGHT / 2.0f;
    corners[2].x = CapsuleObstacle::getWidth() / 2.0f;
    corners[2].y = -HEIGHT / 2.0f;
    corners[3].x = CapsuleObstacle::getWidth() / 2.0f;
    corners[3].y = HEIGHT;

    b2PolygonShape sensorShape;
    sensorShape.Set(corners, 4);

    _hitbox_sensor_def.shape = &sensorShape;
    _hitbox_sensor = _body->CreateFixture(&_hitbox_sensor_def);
  }

  if (_damage_sensor == nullptr) {
    _damage_sensor_def.density = 0.0f;
    _damage_sensor_def.isSensor = true;
    _damage_sensor_name = std::make_shared<std::string>("enemy_damage");
    _damage_sensor_def.userData.pointer =
        reinterpret_cast<uintptr_t>(_damage_sensor_name.get());

    // Sensor dimensions
    b2Vec2 corners[4];
    corners[0].x = -CapsuleObstacle::getWidth() / 2.0f;
    corners[0].y = CapsuleObstacle::getHeight() / 2.0f;
    corners[1].x = -CapsuleObstacle::getWidth() / 2.0f;
    corners[1].y = -CapsuleObstacle::getHeight() / 2.0f;
    corners[2].x = CapsuleObstacle::getWidth() / 2.0f;
    corners[2].y = -CapsuleObstacle::getHeight() / 2.0f;
    corners[3].x = CapsuleObstacle::getWidth() / 2.0f;
    corners[3].y = CapsuleObstacle::getHeight() / 2.0f;

    b2PolygonShape sensorShape;
    sensorShape.Set(corners, 4);

    _damage_sensor_def.shape = &sensorShape;
    _damage_sensor = _body->CreateFixture(&_damage_sensor_def);
  }
}

void EnemyModel::releaseFixtures() {
  if (_body == nullptr) return;

  CapsuleObstacle::releaseFixtures();
  if (_hitbox_sensor != nullptr) {
    _body->DestroyFixture(_hitbox_sensor);
    _hitbox_sensor = nullptr;
  }

  if (_damage_sensor != nullptr) {
    _body->DestroyFixture(_damage_sensor);
    _damage_sensor = nullptr;
  }
}

void EnemyModel::resetSensors() {
  setSensor(false);
  _hitbox_sensor->SetSensor(true);
  _damage_sensor->SetSensor(true);
}

void EnemyModel::update(float delta) {
  CapsuleObstacle::update(delta);
  if (_enemy_node != nullptr) {
    _enemy_node->setPosition(getPosition() + _offset_from_center - _room_pos);
  }

  if (_damage_count <= 0) {
    _enemy_node->setColor(cugl::Color4::WHITE);
    _damage_count = 0;
  } else {
    _damage_count--;
  }

  if (_isKnockbacked) {
    _stunned_timer++;
    if (_stunned_timer >= 10) {
      CULog("finished");
      _isKnockbacked = false;
      _stunned_timer = 0;
    }
  }
}

#pragma mark Movement

void EnemyModel::move(float forwardX, float forwardY) {
  if (_isKnockbacked) {
    setVX(200 * _knockback_dir.x);
    setVY(200 * _knockback_dir.y);

  } else {
    setVX(80 * forwardX);
    setVY(80 * forwardY);
  }

  if (forwardX == 0) {
    setVX(0);
  } else {
    //    setFacingLeft(forwardX < 0 && std::abs(forwardX) > 0.02);
  }

  if (forwardY == 0) setVY(0);
}

void EnemyModel::knockback(int moveDir) {
  _isKnockbacked = true;
  if (moveDir == 0) {
    _knockback_dir.x = -1;
    _knockback_dir.y = 0;
  } else if (moveDir == 1) {
    _knockback_dir.x = 0;
    _knockback_dir.y = -1;
  } else if (moveDir == 2) {
    _knockback_dir.x = 1;
    _knockback_dir.y = 0;
  } else {
    _knockback_dir.x = 0;
    _knockback_dir.y = 1;
  }
}

void EnemyModel::setFacingLeft(bool facing_left) {
  // flip texture if direction has changed
  if (_facing_left != facing_left) {
    _facing_left = facing_left;
    _enemy_node->flipHorizontal(_facing_left);
  }
}
