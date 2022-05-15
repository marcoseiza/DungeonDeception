#include "ShotgunnerController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define MOVE_BACK_RANGE 50
#define ATTACK_RANGE 150
#define ATTACK_FRAMES 20
#define STOP_ATTACK_FRAMES 40
#define ATTACK_COOLDOWN 100
#define MOVE_BACK_COOLDOWN 60
#define WANDER_COOLDOWN 500

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
  if (enemy->getAttackCooldown() <= STOP_ATTACK_FRAMES) {
    enemy->move(0, 0);
    if (enemy->getAttackCooldown() == STOP_ATTACK_FRAMES) {
      enemy->_attack_dir = p;
    }
    if (enemy->getAttackCooldown() == ATTACK_FRAMES) {
      enemy->addBullet(enemy->_attack_dir);
      _sound_controller->playEnemySmallGunshot();
    }
    if (enemy->getAttackCooldown() <= 0) {
      std::uniform_int_distribution<int> dist(0.0f, 50.0f);
      enemy->setAttackCooldown(dist(_generator) + ATTACK_COOLDOWN);
    }
  } else {
    enemy->_attack_dir = p;
    cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
    diff.normalize();
    diff.add(_direction);
    diff.scale(0.6 * enemy->getSpeed());  // Make speed slower when strafing
    enemy->move(diff.x, diff.y);
  }
}

void ShotgunnerController::changeStateIfApplicable(
    std::shared_ptr<EnemyModel> enemy, float distance) {
  // Change state if applicable
  if (distance <= ATTACK_RANGE) {
    std::uniform_int_distribution<int> dist(0, 50);
    if (enemy->getCurrentState() == EnemyModel::State::CHASING) {
      enemy->setAttackCooldown(dist(_generator) + ATTACK_COOLDOWN);
      enemy->_cta_timer++;
    }
    // When timer is over
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
        // Chance is 1/25 for every tick, +10 to attack frames to ensure does not attack in backwards direction
        if (distance <= MOVE_BACK_RANGE && chance <= 1 && enemy->getAttackCooldown() > STOP_ATTACK_FRAMES + 10) {
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
    // Enemy first wanders back to spawn position until it changes to be idle.
    if (enemy->_wander_timer <= 0) {
      enemy->setCurrentState(EnemyModel::State::IDLE);
    }
    if (enemy->getCurrentState() != EnemyModel::State::IDLE && enemy->getCurrentState() != EnemyModel::State::WANDER) {
      enemy->setCurrentState(EnemyModel::State::WANDER);
      enemy->_wander_timer = WANDER_COOLDOWN; // Spends 6 seconds trying to return to original position
    }
    enemy->_wander_timer--;
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

void ShotgunnerController::animate(std::shared_ptr<EnemyModel> enemy,
                                   cugl::Vec2 p) {
  auto node = std::dynamic_pointer_cast<cugl::scene2::SpriteNode>(
      enemy->getNode()->getChildByTag(0));
  auto gun_node = enemy->getNode()->getChildByTag(1);
  int fc = enemy->_frame_count;
  switch (enemy->getCurrentState()) {
    case EnemyModel::State::ATTACKING: {
      if (enemy->getAttackCooldown() <= ATTACK_FRAMES - 5) {
        float angle_inc =
            cugl::Vec2(enemy->_attack_dir - enemy->getPosition()).getAngle() /
            15;
        gun_node->setAngle(angle_inc * enemy->getAttackCooldown());
        if (enemy->getAttackCooldown() == 0) {
          gun_node->setAngle(0);
          gun_node->setVisible(false);
        }
        break;
      } else if (enemy->getAttackCooldown() >= ATTACK_FRAMES + 5 &&
                 enemy->getAttackCooldown() <= STOP_ATTACK_FRAMES) {
        // Here, move up to the desired position
        node->setFrame(1);
        gun_node->setVisible(true);
        float angle_inc =
            cugl::Vec2(enemy->_attack_dir - enemy->getPosition()).getAngle() /
            15;
        gun_node->setAngle(angle_inc *
                           (STOP_ATTACK_FRAMES - enemy->getAttackCooldown()));
        break;
      } else if (enemy->getAttackCooldown() < ATTACK_FRAMES + 5 &&
                 enemy->getAttackCooldown() > ATTACK_FRAMES - 5) {
        break;
      }
    }
    case EnemyModel::State::WANDER:
    case EnemyModel::State::MOVING_BACK:
    case EnemyModel::State::CHASING: {
      gun_node->setVisible(false);
      int run_high_lim = 19;
      int run_low_lim = 10;

      if (fc == 0 || node->getFrame() < run_low_lim) {
        node->setFrame(run_low_lim);
      }

      // Play the next animation frame.
      if (fc >= 4) {
        enemy->_frame_count = 0;
        if (node->getFrame() >= run_high_lim) {
          node->setFrame(run_low_lim);
        } else {
          node->setFrame(node->getFrame() + 1);
        }
      }
      enemy->_frame_count++;
      break;
    }
    default: {
      node->setFrame(0);
      enemy->_frame_count = 0;
      break;
    }
  }
}
