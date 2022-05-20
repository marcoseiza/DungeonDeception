#include "EnemyModel.h"

#include "../controllers/CollisionFiltering.h"

#define WIDTH_GRUNT 24.0f
#define HEIGHT_GRUNT 48.0f
#define WIDTH_TANK 24.0f
#define HEIGHT_TANK 48.0f
#define WIDTH_SHOTGUNNER 24.0f
#define HEIGHT_SHOTGUNNER 48.0f
#define WIDTH_TURTLE 40.0f
#define HEIGHT_TURTLE 35.0f

#define DAMAGE_COUNT 10

#define HEIGHT_SHRINK 0.3f

#pragma mark Init
bool EnemyModel::init(const cugl::Vec2 pos, string name, string type) {
  setName(name);
  setType(type);

  cugl::Vec2 pos_ = pos;

  switch (_enemy_type) {
    case GRUNT:
      _size.set(WIDTH_GRUNT, HEIGHT_GRUNT);
      break;
    case TANK:
      _size.set(WIDTH_TANK, HEIGHT_TANK);
      break;
    case SHOTGUNNER:
      _size.set(WIDTH_SHOTGUNNER, HEIGHT_SHOTGUNNER);
      break;
    case TURTLE:
      _size.set(WIDTH_TURTLE, HEIGHT_TURTLE);
      break;
  }

  _offset_from_center.y = _size.y / 2.0f * (1 - HEIGHT_SHRINK / 2.0f);
  pos_ -= _offset_from_center;

  CapsuleObstacle::init(pos_, _size);

  _init_pos = pos;
  _enemy_node = nullptr;
  _health = 100;
  _facing_left = false;
  _atc_timer = 0;
  _cta_timer = 0;
  _isKnockbacked = false;
  _stunned_timer = 0;
  _goal_frame = 0;
  _move_back_timer = 0;

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

  _wall_fixture_def.filter.categoryBits = CATEGORY_ENEMY;
  _wall_fixture_def.filter.maskBits = MASK_ENEMY_WALL;

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
  _damage_count = DAMAGE_COUNT;
}

bool EnemyModel::isHit() const { return _damage_count == DAMAGE_COUNT - 1; }

void EnemyModel::addBullet(const cugl::Vec2 p) {
  int speed = 300;       // Default speed
  int live_frames = 42;  // Default frames

  if (_enemy_type == SHOTGUNNER) {
    speed = 200;
    live_frames = 100;
    // Shoot two more projectiles on the sides
    cugl::Vec2 p2 = p - getPosition();
    p2.normalize();
    p2.rotate(M_PI / 6);
    auto proj2 = Projectile::alloc(
        cugl::Vec2(getPosition().x, getPosition().y + _offset_from_center.y),
        p2, speed, live_frames);
    _projectiles.emplace(proj2);
    proj2->setPosition(
        cugl::Vec2(getPosition().x, getPosition().y + _offset_from_center.y));

    cugl::Vec2 p3 = p - getPosition();
    p3.normalize();
    p3.rotate(-M_PI / 6);
    auto proj3 = Projectile::alloc(
        cugl::Vec2(getPosition().x, getPosition().y + _offset_from_center.y),
        p3, speed, live_frames);
    _projectiles.emplace(proj3);
    proj3->setPosition(
        cugl::Vec2(getPosition().x, getPosition().y + _offset_from_center.y));
  }

  cugl::Vec2 diff = p - getPosition();
  diff.normalize();
  auto bullet = Projectile::alloc(
      cugl::Vec2(getPosition().x, getPosition().y + _offset_from_center.y),
      diff, speed, live_frames);

  _projectiles.emplace(bullet);
  bullet->setPosition(
      cugl::Vec2(getPosition().x, getPosition().y + _offset_from_center.y));
}

void EnemyModel::deleteProjectile(
    std::shared_ptr<cugl::physics2::ObstacleWorld> _world,
    std::shared_ptr<cugl::scene2::SceneNode> _world_node) {
  auto itt = _projectiles.begin();
  while (itt != _projectiles.end()) {
    if ((*itt)->getFrames() <= 0 || (*itt)->shouldExpire()) {
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
    _speed = 0.65;
  } else if (type == "tank") {
    _enemy_type = TANK;
    _speed = 0.75;
  } else if (type == "turtle") {
    _enemy_type = TURTLE;
    _speed = 0;
  }
}

#pragma mark Animation & Drawing

void EnemyModel::setNode(const std::shared_ptr<cugl::Texture>& texture,
                         std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
  switch (_enemy_type) {
    case SHOTGUNNER: {
      _enemy_node = cugl::scene2::OrderedNode::allocWithOrder(
          cugl::scene2::OrderedNode::Order::ASCEND);
      auto enemy_node = cugl::scene2::SpriteNode::alloc(texture, 9, 10);
      enemy_node->setTag(0);
      enemy_node->setPriority(0);
      enemy_node->setPosition(0, 0);
      _enemy_node->addChild(enemy_node);
      auto gun_node = cugl::scene2::SpriteNode::alloc(texture, 9, 10);
      gun_node->setFrame(2);
      gun_node->setTag(1);
      gun_node->setPriority(1);
      gun_node->setPosition(0, 0);
      gun_node->setVisible(false);
      gun_node->setAnchor(0.42, 0.63);
      _enemy_node->addChild(gun_node);
      break;
    }
    case TANK: {
      _enemy_node = cugl::scene2::SpriteNode::alloc(texture, 7, 10);
      break;
    }
    case GRUNT: {
      _enemy_node = cugl::scene2::SpriteNode::alloc(texture, 7, 10);
      break;
    }
    default: {
      _enemy_node = cugl::scene2::SpriteNode::alloc(texture, 3, 16);
      auto node = dynamic_cast<cugl::scene2::SpriteNode*>(_enemy_node.get());
      node->setFrame(23);  // Initial closed frame
      break;
    }
  }

  // Add the ray cast weights to the debug node.
  for (std::shared_ptr<cugl::scene2::PolygonNode> poly : _polys) {
    debug_node->addChild(poly);
  }
}

/**
 * Sets the scene graph node representing this enemy death.
 * @param node  The scene graph node representing this enemy death.
 */
void EnemyModel::setDeathNode(const std::shared_ptr<cugl::Texture>& texture) {
  switch (_enemy_type) {
    case SHOTGUNNER: {
      _enemy_death_node = cugl::scene2::SpriteNode::alloc(texture, 1, 25);
      break;
    }
    case TANK: {
      _enemy_death_node = cugl::scene2::SpriteNode::alloc(texture, 1, 20);
      break;
    }
    case GRUNT: {
      _enemy_death_node = cugl::scene2::SpriteNode::alloc(texture, 1, 20);
      break;
    }
    case TURTLE: {
      _enemy_death_node = cugl::scene2::SpriteNode::alloc(texture, 1, 23);
      break;
    }
    default:
      break;
  }
  if (_enemy_death_node) _enemy_death_node->setVisible(false);
}

std::shared_ptr<cugl::scene2::SceneNode>& EnemyModel::getNode() {
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
    corners[0].y = _size.y;
    corners[1].x = -CapsuleObstacle::getWidth() / 2.0f;
    corners[1].y = -_size.y / 2.0f;
    corners[2].x = CapsuleObstacle::getWidth() / 2.0f;
    corners[2].y = -_size.y / 2.0f;
    corners[3].x = CapsuleObstacle::getWidth() / 2.0f;
    corners[3].y = _size.y;

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

  if (_wall_fixture == nullptr) {
    _wall_fixture_def.density = 0.0f;
    _wall_fixture_def.isSensor = false;

    // Dimensions
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

    _wall_fixture_def.shape = &sensorShape;
    _wall_fixture = _body->CreateFixture(&_wall_fixture_def);
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

  if (_wall_fixture != nullptr) {
    _body->DestroyFixture(_wall_fixture);
    _wall_fixture = nullptr;
  }
}

void EnemyModel::setAttackingFilter() {
  setSensor(true);
  _wall_fixture->SetSensor(false);
}

void EnemyModel::resetSensors() {
  setSensor(false);
  _damage_sensor->SetSensor(true);
  _hitbox_sensor->SetSensor(true);
}

void EnemyModel::update(float delta) {
  CapsuleObstacle::update(delta);
  if (_enemy_node != nullptr) {
    _enemy_node->setPosition(getPosition() + _offset_from_center - _room_pos);
  }

  if (_damage_count < 0) {
    _enemy_node->setColor(cugl::Color4::WHITE);
    _damage_count = 0;
  } else {
    _damage_count--;
  }

  if (_isKnockbacked) {
    _stunned_timer++;
    if (_stunned_timer >= 10) {
      _isKnockbacked = false;
      _stunned_timer = 0;
    }
  }

  for (auto projectile : _projectiles) {
    projectile->setLifetime(projectile->getLifetime() + 1);
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
