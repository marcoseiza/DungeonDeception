#include "ShotgunnerController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 150
#define TANK_RANGE 30

#define STATE_CHANGE_LIM 10

#pragma mark Shotgunner Controller

ShotgunnerController::ShotgunnerController(){};

bool ShotgunnerController::init(
    std::shared_ptr<cugl::AssetManager> assets,
    std::shared_ptr<cugl::physics2::ObstacleWorld> world,
    std::shared_ptr<cugl::scene2::SceneNode> world_node,
    std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
  _projectile_texture = assets->get<cugl::Texture>("projectile-red-large");
  EnemyController::init(assets, world, world_node, debug_node);

  return true;
}

void ShotgunnerController::attackPlayer(std::shared_ptr<EnemyModel> enemy,
                                  cugl::Vec2 p) {
  if (enemy->getAttackCooldown() <= 30) {
    enemy->move(0,0);
    if (enemy->getAttackCooldown() == 20) {
      enemy->addBullet(p);
    }
    if (enemy->getAttackCooldown() <= 0) {
      enemy->setAttackCooldown(rand() % 50 + 155);
    }
  } else {
    cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
    diff.normalize();
    diff.add(_direction);
    diff.scale(0.6 * enemy->getSpeed()); // Make speed slower when strafing
    enemy->move(diff.x, diff.y);
  }
}


void ShotgunnerController::changeStateIfApplicable(
    std::shared_ptr<EnemyModel> enemy, float distance) {
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

void ShotgunnerController::performAction(std::shared_ptr<EnemyModel> enemy,
                                         cugl::Vec2 p) {
  switch (enemy->getCurrentState()) {
    case EnemyModel::State::CHASING: {
      chasePlayer(enemy, p);
      break;
    }
    case EnemyModel::State::ATTACKING: {
      attackPlayer(enemy, p);
      break;
    }
    default: {
      idling(enemy);
    }
  }
}
