#include "TankController.h"

#define MIN_DISTANCE 300
#define MOVE_BACK_RANGE 40
#define ATTACK_RANGE 75
#define ATTACK_FRAMES 18
#define STOP_ATTACK_FRAMES 50
#define ATTACK_COOLDOWN 120
#define MOVE_BACK_COOLDOWN 60

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
      _sound_controller->playEnemySwing();
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
    std::uniform_int_distribution<int> dist(0, 50);
    if (enemy->getCurrentState() == EnemyModel::State::CHASING) {
      enemy->setAttackCooldown(dist(_generator) + ATTACK_COOLDOWN);
      enemy->_cta_timer++;
    }
    if (enemy->_cta_timer == 0 || enemy->_cta_timer == STATE_CHANGE_LIM) {
      enemy->_cta_timer = 0;
      if (enemy->getCurrentState() == EnemyModel::State::MOVING_BACK) {
        enemy->_move_back_timer--;
        if (enemy->_move_back_timer <= 0) {
          enemy->setAttackCooldown(dist(_generator) + ATTACK_COOLDOWN);
          enemy->setCurrentState(EnemyModel::State::ATTACKING);
        }
      } else {
        // Chance for the enemy to move backwards, away from the player.
        int chance = dist(_generator);
        // Chance is 1/20 for every tick, +10 to attack frames to ensure does not attack in backwards direction
        if (distance <= MOVE_BACK_RANGE && chance <= 5 && enemy->getAttackCooldown() > STOP_ATTACK_FRAMES + 10) {
          enemy->setCurrentState(EnemyModel::State::MOVING_BACK);
          enemy->_move_back_timer = MOVE_BACK_COOLDOWN;
          enemy->setAttackCooldown(ATTACK_COOLDOWN);
        } else {
          enemy->setCurrentState(EnemyModel::State::ATTACKING);
        }
      }
    }
  } else if (distance <= MIN_DISTANCE) {
    if (enemy->getCurrentState() != EnemyModel::State::CHASING) {
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
    case EnemyModel::State::MOVING_BACK: {
      avoidPlayer(enemy, p);
      break;
    }
    default: {
      idling(enemy);
    }
  }
}

void TankController::animate(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p) {}
