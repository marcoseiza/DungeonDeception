#include "TurtleController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 150
#define TANK_RANGE 100

#pragma mark Turtle Controller

TurtleController::TurtleController(){};

bool TurtleController::init(
    std::shared_ptr<cugl::AssetManager> assets,
    std::shared_ptr<cugl::physics2::ObstacleWorld> world,
    std::shared_ptr<cugl::scene2::SceneNode> world_node,
    std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
  _projectile_texture = assets->get<cugl::Texture>("projectile-orange-large");
  EnemyController::init(assets, world, world_node, debug_node);

  return true;
}

void TurtleController::attackPlayer(std::shared_ptr<EnemyModel> enemy,
                                    const cugl::Vec2 p) {
  if (enemy->getAttackCooldown() <= 0) {
    cugl::Vec2 e = enemy->getPosition();

    // Attack in closest cardinal direction
    if (abs(p.x - e.x) > abs(p.y - e.y)) {
      int add = (p.x - e.x > 0) ? 1 : -1;
      enemy->addBullet(cugl::Vec2(e.x + add, e.y));
    } else {
      int add = (p.y - e.y > 0) ? 1 : -1;
      enemy->addBullet(cugl::Vec2(e.x, e.y + add));
    }
    enemy->setAttackCooldown(120);
  }
  enemy->move(0, 0);
}

void TurtleController::tank(std::shared_ptr<EnemyModel> enemy) {
  enemy->move(0, 0);
}

void TurtleController::changeStateIfApplicable(
    std::shared_ptr<EnemyModel> enemy, float distance) {
  if (distance <= TANK_RANGE) {
    enemy->setCurrentState(EnemyModel::State::TANKING);
  } else if (distance <= ATTACK_RANGE) {
    enemy->setCurrentState(EnemyModel::State::ATTACKING);
  } else {
    enemy->setCurrentState(EnemyModel::State::IDLE);
  }
}

void TurtleController::performAction(std::shared_ptr<EnemyModel> enemy,
                                     cugl::Vec2 p) {
  switch (enemy->getCurrentState()) {
    case EnemyModel::State::ATTACKING: {
      attackPlayer(enemy, p);
      break;
    }
    case EnemyModel::State::TANKING: {
      tank(enemy);
      break;
    }
    default: {
      idling(enemy);
    }
  }
}

void TurtleController::animate(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p) {
  auto node = dynamic_cast<cugl::scene2::SpriteNode*>(enemy->getNode().get());
  int fc = enemy->_frame_count;
  switch (enemy->getCurrentState()) {
    case EnemyModel::State::ATTACKING: {
      node->setFrame(0);
      break;
    }
    case EnemyModel::State::TANKING: {
      node->setFrame(1);
      break;
    }
    default: {
      node->setFrame(0);
    }
  }
}
