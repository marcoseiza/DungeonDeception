#include "TankController.h"

#define MIN_DISTANCE 300
#define MOVE_BACK_RANGE 40
#define ATTACK_RANGE 75
#define ATTACK_FRAMES 18
#define STOP_ATTACK_FRAMES 50
#define ATTACK_COOLDOWN 120
#define MOVE_BACK_COOLDOWN 60
#define WANDER_COOLDOWN 500

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

void TankController::clientUpdateAttackPlayer(std::shared_ptr<EnemyModel> enemy) {
  if (enemy->didAttack()) {
    // Begin holding.
    enemy->setAttackCooldown(STOP_ATTACK_FRAMES);
    enemy->setAttack(false);
  } else if (enemy->getAttackCooldown() >= -5 && enemy->getAttackCooldown() != ATTACK_COOLDOWN - 1) {
    // Whether the enemy is attacking
    if (enemy->getAttackCooldown() == -5) {
      // Attack done.
      enemy->resetSensors();
      enemy->setAttackCooldown(ATTACK_COOLDOWN);
    }
    if (enemy->getAttackCooldown() == ATTACK_FRAMES) {
      // Begin actual dash.
      enemy->setAttackingFilter();
      _sound_controller->playEnemySwing();
    }
    enemy->reduceAttackCooldown(1);
  }
}

void TankController::attackPlayer(std::shared_ptr<EnemyModel> enemy,
                                  cugl::Vec2 p) {
  if (enemy->getAttackCooldown() <= 0) {
    // Just finished attack, reset attack cooldown.
    std::uniform_int_distribution<int> dist(0.0f, 50.0f);
    enemy->setAttackCooldown(dist(_generator) + ATTACK_COOLDOWN);
    enemy->resetSensors();
  } else if (enemy->getAttackCooldown() < ATTACK_FRAMES) {
    // Currently attacking player.
    cugl::Vec2 dir = enemy->getAttackDir() - enemy->getAttackInitPos();
    dir.normalize();
    dir.scale(3.5);
    enemy->move(dir.x, dir.y);
  } else if (enemy->getAttackCooldown() == ATTACK_FRAMES) {
    // Begin attack.
    enemy->setAttackingFilter();
    _sound_controller->playEnemySwing();
  } else if (enemy->getAttackCooldown() < STOP_ATTACK_FRAMES) {
    // Stops in place to wind up attack.
    enemy->move(0, 0);
  } else if (enemy->getAttackCooldown() == STOP_ATTACK_FRAMES) {
    // Determine attack position.
    enemy->setAttack(true);
    enemy->setAttackDir(p);
    enemy->setAttackInitPos(enemy->getPosition());
  } else {
    // Circle the player.
    enemy->setAttackDir(p);
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
    std::uniform_int_distribution<int> dist(0, 50);
    if (enemy->getCurrentState() == EnemyModel::State::MOVING_BACK) {
      enemy->_move_back_timer--;
      if (enemy->_move_back_timer <= 0) {
        enemy->setAttackCooldown(STOP_ATTACK_FRAMES + 10);
        enemy->setCurrentState(EnemyModel::State::ATTACKING);
      }
    } else {
      // Chance for the enemy to move backwards, away from the player.
      int chance = dist(_generator);
      // Chance is 1/25 for every tick; +10 to attack frames to ensure does not attack in backwards direction
      if (distance <= MOVE_BACK_RANGE && chance <= 1 && enemy->getAttackCooldown() > STOP_ATTACK_FRAMES + 10) {
        enemy->setCurrentState(EnemyModel::State::MOVING_BACK);
        enemy->_move_back_timer = MOVE_BACK_COOLDOWN;
        enemy->setAttackCooldown(ATTACK_COOLDOWN);
      } else {
        enemy->setCurrentState(EnemyModel::State::ATTACKING);
      }
    }
  } else if (distance <= MIN_DISTANCE) {
    // Only change to chasing if not currently attacking.
    if (enemy->getCurrentState() != EnemyModel::State::ATTACKING || enemy->getAttackCooldown() > STOP_ATTACK_FRAMES) {
      enemy->setCurrentState(EnemyModel::State::CHASING);
      enemy->setAttackCooldown(ATTACK_COOLDOWN);
    }
  } else {
    // Enemy first wanders back to spawn position until it changes to be idle.
    if (enemy->_wander_timer <= 0) {
      enemy->setCurrentState(EnemyModel::State::IDLE);
    }
    if (enemy->getCurrentState() != EnemyModel::State::IDLE && enemy->getCurrentState() != EnemyModel::State::WANDER) {
      enemy->setCurrentState(EnemyModel::State::WANDER);
      enemy->_wander_timer = WANDER_COOLDOWN; // Spends 6 seconds trying to return to original position
    }
    enemy->setAttackCooldown(ATTACK_COOLDOWN);
    enemy->_wander_timer--;
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
    case EnemyModel::State::MOVING_BACK: {
      avoidPlayer(enemy, p);
      break;
    }
    case EnemyModel::State::WANDER: {
      moveBackToOriginalSpot(enemy);
      break;
    }
    default: {
      idling(enemy, p);
    }
  }
}

void TankController::animate(std::shared_ptr<EnemyModel> enemy) {}
