#include "TankController.h"

#define MIN_DISTANCE 300
#define ATTACK_RANGE 75
#define ATTACK_FRAMES 18
#define STOP_ATTACK_FRAMES 50
#define ATTACK_COOLDOWN 120

#define STATE_CHANGE_LIM 10

#pragma mark Tank Controller

TankController::TankController(){};

bool TankController::init(std::shared_ptr<cugl::AssetManager> assets,
                          std::shared_ptr<cugl::physics2::ObstacleWorld> world,
                          std::shared_ptr<cugl::scene2::SceneNode> world_node,
                          std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
  _projectile_texture = assets->get<cugl::Texture>("projectile-red-large");
  EnemyController::init(assets, world, world_node, debug_node);

  return true;
}

void TankController::attackPlayer(std::shared_ptr<EnemyModel> enemy,
                                  cugl::Vec2 p) {
  if (enemy->getAttackCooldown() <= ATTACK_FRAMES) {
    if (enemy->getAttackCooldown() == ATTACK_FRAMES) {
      enemy->setSensor(true);
    }
    if (enemy->getAttackCooldown() <= 0) {
      std::uniform_int_distribution<int> dist(0.0f, 50.0f);
      enemy->setAttackCooldown(dist(_generator) + ATTACK_COOLDOWN);
      enemy->resetSensors();
    } else {
      cugl::Vec2 dir = enemy->_attack_dir;
      dir.scale(5);
      enemy->move(dir.x, dir.y);
    }
  } else if (enemy->getAttackCooldown() <= STOP_ATTACK_FRAMES) {
    enemy->move(0, 0);
    if (enemy->getAttackCooldown() == STOP_ATTACK_FRAMES) {
      enemy->_attack_dir = p - enemy->getPosition();
      enemy->_attack_dir.normalize();
    }
  } else {
    cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
    diff.normalize();
    diff.add(_direction);
    diff.scale(0.6 * enemy->getSpeed());
    enemy->move(diff.x, diff.y);
  }
}

void TankController::changeStateIfApplicable(std::shared_ptr<EnemyModel> enemy,
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

void TankController::performAction(std::shared_ptr<EnemyModel> enemy,
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

void TankController::animate(std::shared_ptr<EnemyModel> enemy) {}
