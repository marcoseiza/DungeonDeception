#include "TankController.h"

#define MIN_DISTANCE 300
#define ATTACK_RANGE 30

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
}

void TankController::changeStateIfApplicable(std::shared_ptr<EnemyModel> enemy,
                                             float distance) {
  // Change state if applicable
  if (distance <= ATTACK_RANGE) {
    enemy->setCurrentState(EnemyModel::State::ATTACKING);
  } else if (distance <= MIN_DISTANCE) {
    enemy->setCurrentState(EnemyModel::State::CHASING);
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
