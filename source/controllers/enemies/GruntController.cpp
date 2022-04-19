#include "GruntController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 75

#define STATE_CHANGE_LIM 10

#pragma mark GruntController

void GruntController::attackPlayer(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p) {
  if (enemy->getAttackCooldown() <= 18) {
    if (enemy->getAttackCooldown() == 18) {
      enemy->_attack_dir = p - enemy->getPosition();
      enemy->_attack_dir.normalize();
      enemy->setSensor(true);
    } 
    cugl::Vec2 dir = enemy->_attack_dir;
    dir.scale(5);
    enemy->move(dir.x, dir.y);
    if (enemy->getAttackCooldown() <= 0) {
      enemy->setAttackCooldown(rand() % 50 + 155);
      enemy->resetSensors();
    }
  } else if (enemy->getAttackCooldown() <= 45) {
    enemy->move(0, 0);
  } else {
    cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
    diff.normalize();
    diff.add(_direction);
    diff.scale(0.6);
    enemy->move(diff.x, diff.y);
  }
}

bool GruntController::init(
    std::shared_ptr<cugl::AssetManager> assets,
    std::shared_ptr<cugl::physics2::ObstacleWorld> world,
    std::shared_ptr<cugl::scene2::SceneNode> world_node,
    std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
  _projectile_texture = assets->get<cugl::Texture>("projectile-blue-large");
  EnemyController::init(assets, world, world_node, debug_node);

  return true;
}

void GruntController::changeStateIfApplicable(std::shared_ptr<EnemyModel> enemy,
                                              float distance) {
  // Change state if applicable
  if (distance <= ATTACK_RANGE) {
    if (enemy->getCurrentState() == EnemyModel::State::CHASING) {
      enemy->_cta_timer++;
    }
    if (enemy->_cta_timer == 0 || enemy->_cta_timer == STATE_CHANGE_LIM) {
      enemy->setCurrentState(EnemyModel::State::ATTACKING);
      enemy->_cta_timer = 0;
    }
  } else if (distance <= MIN_DISTANCE) {
    if (enemy->getCurrentState() == EnemyModel::State::ATTACKING) {
      enemy->_atc_timer++;
    }
    if (enemy->_atc_timer == 0 || enemy->_atc_timer == STATE_CHANGE_LIM) {
      enemy->setCurrentState(EnemyModel::State::CHASING);
      enemy->_atc_timer = 0;
    }
  } else {
    enemy->setCurrentState(EnemyModel::State::IDLE);
  }
}

void GruntController::performAction(std::shared_ptr<EnemyModel> enemy,
                                    cugl::Vec2 p) {
  switch (enemy->getCurrentState()) {
    case EnemyModel::State::IDLE: {
      idling(enemy);
      break;
    }
    case EnemyModel::State::CHASING: {
      chasePlayer(enemy, p);
      break;
    }
    case EnemyModel::State::ATTACKING: {
      attackPlayer(enemy, p);
      break;
    }
    default: {
      avoidPlayer(enemy, p);
      break;
    }
  }
}
