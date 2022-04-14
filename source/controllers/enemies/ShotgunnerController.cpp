#include "ShotgunnerController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 150
#define TANK_RANGE 30

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

void ShotgunnerController::skirtPlayer(std::shared_ptr<EnemyModel> enemy,
                                       cugl::Vec2 p) {
  // Basically avoid the player, but slower
  cugl::Vec2 diff = p - enemy->getPosition();
  diff.subtract(enemy->getVX(), enemy->getVY());
  diff.add(enemy->getVX(), enemy->getVY());
  diff.scale(enemy->getSpeed() / 3);
  enemy->move(-diff.x, -diff.y);

  // Attack enemy if you can
  if (enemy->getAttackCooldown() <= 0) {
    enemy->addBullet(p);
    enemy->setAttackCooldown(120);
  }
}

void ShotgunnerController::changeStateIfApplicable(
    std::shared_ptr<EnemyModel> enemy, float distance) {
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
    case EnemyModel::State::SKIRTING: {
      skirtPlayer(enemy, p);
      break;
    }
    default: {
      idling(enemy);
    }
  }
}
