#include "GruntController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 100

#pragma mark GruntController

void GruntController::attackPlayer(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p) {
  if (enemy->getAttackCooldown() <= 0) {
    // TODO: Make the grunt dash towards the player instead of firing a bullet.
    enemy->addBullet(p);
    enemy->setAttackCooldown(120);
  } else {
    cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
    diff.normalize();
    diff.add(_direction);
    diff.scale(enemy->getSpeed()*0.75);
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
    enemy->setCurrentState(EnemyModel::State::ATTACKING);
  } else if (distance <= MIN_DISTANCE) {
    if (enemy->getCurrentState() == EnemyModel::State::ATTACKING) {
      enemy->_atc_timer++;
    }
    if (enemy->_atc_timer == 0 || enemy->_atc_timer == 10) {
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
