#include "TankController.h"

#define MIN_DISTANCE 300
#define ATTACK_RANGE 50

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
  // TODO: add melee fixture onto the tank, for now just attack by hitting body
  cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
  diff.normalize();
  diff.add(_direction);
  diff.scale(enemy->getSpeed());
  enemy->move(diff.x, diff.y);
}

void TankController::changeStateIfApplicable(std::shared_ptr<EnemyModel> enemy,
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
