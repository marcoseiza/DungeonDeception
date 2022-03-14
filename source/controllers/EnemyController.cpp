#include "EnemyController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 100

#pragma mark EnemyController

EnemyController::EnemyController(){};

void EnemyController::idling(std::shared_ptr<Grunt> enemy) {
  enemy->move(0, 0);
}

void EnemyController::chasePlayer(std::shared_ptr<Grunt> enemy,
                                  const cugl::Vec2 p) {
  cugl::Vec2 diff = p - enemy->getPosition();
  diff.subtract(enemy->getVX(), enemy->getVY());
  diff.add(enemy->getVX(), enemy->getVY());
  diff.scale(enemy->getSpeed());
  enemy->move(diff.x, diff.y);
}

void EnemyController::attackPlayer(std::shared_ptr<Grunt> enemy,
                                   const cugl::Vec2 p) {
  if (enemy->getAttackCooldown() <= 0) {
    enemy->addBullet(p);
    enemy->setAttackCooldown(120);
  }
  enemy->move(0, 0);
}

void EnemyController::avoidPlayer(std::shared_ptr<Grunt> enemy,
                                  const cugl::Vec2 p) {
  cugl::Vec2 diff = p - enemy->getPosition();
  diff.subtract(enemy->getVX(), enemy->getVY());
  diff.add(enemy->getVX(), enemy->getVY());
  diff.scale(enemy->getSpeed() / 2);
  enemy->move(-diff.x, -diff.y);
}

bool EnemyController::init(
    std::shared_ptr<cugl::AssetManager> assets,
    std::shared_ptr<cugl::physics2::ObstacleWorld> world,
    std::shared_ptr<cugl::scene2::SceneNode> world_node,
    std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
  _world = world;
  _world_node = world_node;
  _debug_node = debug_node;

  _projectile_texture = assets->get<cugl::Texture>("projectile");

  return true;
}

void EnemyController::update(float timestep, std::shared_ptr<Grunt> enemy,
                             std::shared_ptr<Player> player) {
  // Change state if applicable
  float distance =
      (enemy->getPosition()).subtract(player->getPosition()).length();
  changeStateIfApplicable(enemy, distance);

  // Perform player action
  cugl::Vec2 p = player->getPosition();
  performAction(enemy, p);

  // Reduce attack cooldown if enemy has attacked
  if (enemy->getAttackCooldown() > 0) {
    enemy->reduceAttackCooldown(1);
  }

  // Update enemy & projectiles
  updateProjectiles(timestep, enemy);
  enemy->update(timestep);
}

void EnemyController::changeStateIfApplicable(std::shared_ptr<Grunt> enemy,
                                              float distance) {
  // Change state if applicable
  int health = enemy->getHealth();
  if (health <= HEALTH_LIM) {
    enemy->setCurrentState(Grunt::State::AVOIDING);
    if (distance > MIN_DISTANCE) {
      enemy->setCurrentState(Grunt::State::IDLE);
    }
  } else {
    if (distance <= ATTACK_RANGE) {
      enemy->setCurrentState(Grunt::State::ATTACKING);
    } else if (distance <= MIN_DISTANCE) {
      enemy->setCurrentState(Grunt::State::CHASING);
    } else {
      enemy->setCurrentState(Grunt::State::IDLE);
    }
  }
}

void EnemyController::performAction(std::shared_ptr<Grunt> enemy,
                                    cugl::Vec2 p) {
  switch (enemy->getCurrentState()) {
    case Grunt::State::IDLE: {
      idling(enemy);
      break;
    }
    case Grunt::State::CHASING: {
      chasePlayer(enemy, p);
      break;
    }
    case Grunt::State::ATTACKING: {
      attackPlayer(enemy, p);
      break;
    }
    case Grunt::State::AVOIDING: {
      avoidPlayer(enemy, p);
      break;
    }
  }
}

void EnemyController::updateProjectiles(float timestep,
                                        std::shared_ptr<Grunt> enemy) {
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
