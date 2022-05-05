#include "EnemyController.h"

#include <box2d/b2_world.h>

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 100

#define NUM_RAYCASTS 64
#define RAYCAST_LENGTH 20
#define NUM_WEIGHTS 16

#define MAX_SPEED 0.1f

#pragma mark EnemyController

EnemyController::EnemyController(){};

void EnemyController::idling(std::shared_ptr<EnemyModel> enemy) {
  enemy->move(0, 0);
}

void EnemyController::chasePlayer(std::shared_ptr<EnemyModel> enemy,
                                  const cugl::Vec2 p) {
  // Using steering behavior for smooth movement.
  cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
  diff.normalize();
  diff.add(_direction);
  diff.scale(enemy->getSpeed());

  enemy->move(diff.x, diff.y);
}

void EnemyController::attackPlayer(std::shared_ptr<EnemyModel> enemy,
                                   const cugl::Vec2 p) {
  if (enemy->getAttackCooldown() <= 0) {
    enemy->addBullet(p);
    enemy->setAttackCooldown(120);
  }

  cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
  diff.normalize();
  diff.add(_direction);
  diff.scale(0.6 * enemy->getSpeed());  // Make speed slower when strafing
  enemy->move(diff.x, diff.y);
}

void EnemyController::avoidPlayer(std::shared_ptr<EnemyModel> enemy,
                                  const cugl::Vec2 p) {
  cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
  diff.normalize();
  diff.add(-_direction);
  diff.scale(0.6 * enemy->getSpeed());  // Make speed slower when strafing
  enemy->move(diff.x, diff.y);
}

void EnemyController::stunned(std::shared_ptr<EnemyModel> enemy) {
  enemy->move(10, 10);
}

bool EnemyController::init(
    std::shared_ptr<cugl::AssetManager> assets,
    std::shared_ptr<cugl::physics2::ObstacleWorld> world,
    std::shared_ptr<cugl::scene2::SceneNode> world_node,
    std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
  _world = world;
  _world_node = world_node;
  _debug_node = debug_node;

  std::random_device rd;
  _generator = std::default_random_engine(rd());

  return true;
}

void EnemyController::update(bool is_host, float timestep,
                             std::shared_ptr<EnemyModel> enemy,
                             std::vector<std::shared_ptr<Player>> _players,
                             int room_id) {
  if (!is_host) {
    // Update enemy & projectiles
    updateProjectiles(timestep, enemy);
    enemy->update(timestep);
    return;
  }

  float min_distance = std::numeric_limits<float>::max();
  std::shared_ptr<Player> min_player = _players[0];

  // find closest player to enemy in the same room
  for (std::shared_ptr<Player> player : _players) {
    if (player->getRoomId() == room_id) {
      float distance =
          (enemy->getPosition()).subtract(player->getPosition()).length();
      if (distance < min_distance) {
        min_distance = distance;
        min_player = player;
      }
    }
  }
  // if player not in room id (i.e. no player found, should not happen, return!)
  if (min_player->getRoomId() != room_id) {
    return;
  }

  // Change state if applicable
  float distance =
      (enemy->getPosition()).subtract(min_player->getPosition()).length();
  changeStateIfApplicable(enemy, distance);

  // Figure out direction in which to move in
  findWeights(enemy, min_player);

  // Perform player action
  cugl::Vec2 p = min_player->getPosition();
  performAction(enemy, p);

  // Reduce attack cooldown if enemy has attacked
  if (enemy->getAttackCooldown() > 0) {
    enemy->reduceAttackCooldown(1);
  }

  if (enemy->isHit()) {
    _sound_controller->playEnemyHit();
  }

  // Update enemy & projectiles
  updateProjectiles(timestep, enemy);
  enemy->update(timestep);

  animate(enemy, p);
}

void EnemyController::findWeights(std::shared_ptr<EnemyModel> enemy,
                                  std::shared_ptr<Player> player) {
  // Perform 16 ray casts. For each ray cast, if hit a fixture, adjust the 12
  // weights.
  float theta = 0;
  b2Vec2 p1 = b2Vec2(enemy->getPosition().x, enemy->getPosition().y);
  b2Vec2 p2;
  bool found_player = false;
  for (int i = 0; i < NUM_RAYCASTS; i++) {
    p2 = b2Vec2(p1.x + RAYCAST_LENGTH * cos(theta),
                p1.y + RAYCAST_LENGTH * sin(theta));  // The ray cast end point.
    RayCastController raycast;
    _world->getWorld()->RayCast(&raycast, p1, p2);  // Perform ray cast.
    // Get fixture and body names for checking ray cast hits.
    auto fx = raycast.m_fixture;

    // If ray cast hit a fixture, adjust weights accordingly.
    if (fx) {
      void* fx_d = (void*)fx->GetUserData().pointer;
      std::string fx_name;
      if (static_cast<std::string*>(fx_d) != nullptr)
        fx_name.assign(*static_cast<std::string*>(fx_d));
      b2Body* bd = fx->GetBody();
      cugl::physics2::Obstacle* ob = static_cast<cugl::physics2::Obstacle*>(
          (void*)bd->GetUserData().pointer);

      // If the ray cast hit an object, add it to the map to determine weights
      // after (need set so that it doesn't do  calculations for the same object
      // multiple times). Also need to find the closest distanced ray cast.
      if (ob->getName() == "Wall" || fx_name == "enemy_hitbox") {
        if (_objects.find(ob) != _objects.end()) {
          float rc_diff = cugl::Vec2(raycast.m_point.x - enemy->getPosition().x,
                                     raycast.m_point.y - enemy->getPosition().y)
                              .length();
          float min_diff =
              cugl::Vec2(_objects.at(ob).x - enemy->getPosition().x,
                         _objects.at(ob).y - enemy->getPosition().y)
                  .length();
          if (rc_diff < min_diff) {
            _objects.emplace(ob,
                             cugl::Vec2(raycast.m_point.x, raycast.m_point.y));
          }
        } else {
          _objects.emplace(ob,
                           cugl::Vec2(raycast.m_point.x, raycast.m_point.y));
        }
      }

      // Check if the player is in the circle of vision.
      if (ob == player.get()) {
        found_player = true;
      }
    }
    theta += M_PI / (NUM_RAYCASTS / 2);
  }

  // Adjust the weights (lower them) according to walls and other enemies that
  // were raycasted against.
  for (auto it : _objects) {
    auto ob = it.first;
    auto point = it.second;
    // Wall weight adjustments, done according to point hit to the wall.
    if (ob->getName() == "Wall") {
      cugl::Vec2 ob_vec = point - enemy->getPosition();
      ob_vec.normalize();
      theta = 0;
      for (int i = 0; i < NUM_WEIGHTS; i++) {
        cugl::Vec2 weight_vec = cugl::Vec2(cos(theta), sin(theta));
        float dot = cugl::Vec2::dot(ob_vec, weight_vec);
        _weights[i] -= dot;
        theta += M_PI / (NUM_WEIGHTS / 2);
      }
    } else {  // Enemy weight adjustments, done according to enemy position.
      cugl::Vec2 ob_vec = ob->getPosition() - enemy->getPosition();
      ob_vec.normalize();
      theta = 0;
      for (int i = 0; i < NUM_WEIGHTS; i++) {
        // Move away from the other enemies at an angle.
        cugl::Vec2 weight_vec =
            cugl::Vec2(cos(theta + M_PI / 4), sin(theta + M_PI / 4));
        float dot = cugl::Vec2::dot(ob_vec, weight_vec);
        _weights[i] -= dot;
        theta += M_PI / (NUM_WEIGHTS / 2);
      }
    }
  }

  // Adjust the weights (raise them) according to the player position.
  cugl::Vec2 p = player->getPosition() - enemy->getPosition();
  p.normalize();
  theta = 0;
  for (int i = 0; i < NUM_WEIGHTS; i++) {
    // If attacking, move at a normal instead of directly at the player.
    cugl::Vec2 weight_vec = cugl::Vec2(cos(theta), sin(theta));
    if (enemy->getCurrentState() == EnemyModel::State::ATTACKING) {
      // Determine the angle in which the enemy wants to move, to figure out if
      // it should move CW or CCW.
      int direc = (0 < _weights[i]) - (_weights[i] < 0);
      cugl::Vec2 plus = p + cugl::Vec2(cos(theta) * direc, sin(theta) * direc);
      plus.normalize();
      float angle = cugl::Vec2::angle(p, plus);
      _cw_direcs[i] = angle > 0;
      if (angle == 0) {
        _cw_direcs[i] = enemy->_move_CW;
      }
      int CW = _cw_direcs[i] ? -1 : 1;
      weight_vec =
          cugl::Vec2(cos(theta + CW * M_PI / 2), sin(theta + CW * M_PI / 2));
    }
    float dot = cugl::Vec2::dot(p, weight_vec);
    _weights[i] += dot;
    theta += M_PI / (NUM_WEIGHTS / 2);
  }

  // Adjust weights to slightly prefer current direction.
  theta = 0;
  for (int i = 0; i < NUM_WEIGHTS; i++) {
    cugl::Vec2 direc = cugl::Vec2(cos(theta), sin(theta));
    float dot = cugl::Vec2::dot(direc, _direction);
    if (dot > 0) {
      _weights[i] += dot / 5;
    }
    theta += M_PI / (NUM_WEIGHTS / 2);
  }

  // Find the weight with the highest value, move in that direction.
  // In case of tie, will take the weight in the most CCW direction from theta =
  // 0.
  float highest = _weights[0];
  int highest_ind = 0;
  for (int i = 0; i < NUM_WEIGHTS; i++) {
    if (_weights[i] > _weights[highest_ind]) {
      highest = _weights[i];
      highest_ind = i;
    }
  }
  _direction = cugl::Vec2(cos(highest_ind * M_PI / (NUM_WEIGHTS / 2)),
                          sin(highest_ind * M_PI / (NUM_WEIGHTS / 2)));
  enemy->_move_CW = _cw_direcs[highest_ind];

  // Visualize the weights, if debug mode is on.
  if (_debug_node->isVisible()) {
    theta = 0;
    for (int i = 0; i < NUM_WEIGHTS; i++) {
      cugl::Vec2 p4 =
          cugl::Vec2(enemy->getPosition().x +
                         RAYCAST_LENGTH * cos(theta) * abs(_weights[i]),
                     enemy->getPosition().y +
                         RAYCAST_LENGTH * sin(theta) * abs(_weights[i]));
      cugl::Path2 x = cugl::Path2();
      x.push(enemy->getPosition());
      x.push(p4);
      cugl::SimpleExtruder ex = cugl::SimpleExtruder();
      ex.set(x);
      ex.calculate(1);
      cugl::Poly2 p = ex.getPolygon();
      enemy->_polys.at(i)->setVisible(true);
      enemy->_polys.at(i)->setPolygon(p);
      enemy->_polys.at(i)->setPosition(
          cugl::Vec2(enemy->getPosition().x +
                         (RAYCAST_LENGTH / 2) * cos(theta) * abs(_weights[i]),
                     enemy->getPosition().y +
                         (RAYCAST_LENGTH / 2) * sin(theta) * abs(_weights[i])));
      if (_weights[i] < 0) {
        enemy->_polys.at(i)->setColor(cugl::Color4::RED);
      } else {
        enemy->_polys.at(i)->setColor(cugl::Color4::WHITE);
      }
      if (i == highest_ind) {
        enemy->_polys.at(i)->setColor(cugl::Color4::BLUE);
      }
      theta += M_PI / (NUM_WEIGHTS / 2);
    }
  }

  _objects.clear();
  _weights.fill(0);
  _cw_direcs.fill(false);
}

void EnemyController::updateProjectiles(float timestep,
                                        std::shared_ptr<EnemyModel> enemy) {
  auto proj = enemy->getProjectiles();
  auto it = proj.begin();
  while (it != proj.end()) {
    // Add to world if needed
    if ((*it)->getNode() == nullptr) {
      _world->addObstacle((*it));
      auto proj_node =
          cugl::scene2::SpriteNode::alloc(_projectile_texture, 1, 1);
      proj_node->setPosition((*it)->getPosition());
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
