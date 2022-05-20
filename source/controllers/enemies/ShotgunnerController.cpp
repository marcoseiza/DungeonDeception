#include "ShotgunnerController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define MOVE_BACK_RANGE 50
#define ATTACK_RANGE 150
#define ATTACK_FRAMES 20
#define STOP_ATTACK_FRAMES 80
#define ATTACK_COOLDOWN 120
#define MOVE_BACK_COOLDOWN 60
#define WANDER_COOLDOWN 500
#define GUN_NODE_X_OFFSET 0.42
#define GUN_NODE_Y_OFFSET 0.59
#define GUN_MOVE_FRAMES 15

#define DEATH_RIGHT_LOW_LIM 40
#define DEATH_RIGHT_UP_LIM 64
#define DEATH_LEFT_LOW_LIM 70
#define DEATH_LEFT_UP_LIM 94

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

void ShotgunnerController::clientUpdateAttackPlayer(
    std::shared_ptr<EnemyModel> enemy) {
  if (enemy->didAttack()) {
    // Begin holding.
    enemy->setAttackCooldown(STOP_ATTACK_FRAMES);
    enemy->setAttack(false);
  } else if (enemy->getAttackCooldown() >= 0 &&
             enemy->getAttackCooldown() != ATTACK_COOLDOWN - 1) {
    // Whether the enemy is attacking
    if (enemy->getAttackCooldown() == 0) {
      // Attack done.
      enemy->setAttackCooldown(ATTACK_COOLDOWN);
    }
    if (enemy->getAttackCooldown() == ATTACK_FRAMES) {
      // Add the bullet!
      enemy->addBullet(enemy->getAttackDir());
      _sound_controller->playEnemySmallGunshot();
    }
    enemy->reduceAttackCooldown(1);
  }
}

void ShotgunnerController::attackPlayer(std::shared_ptr<EnemyModel> enemy,
                                        cugl::Vec2 p) {
  if (enemy->getAttackCooldown() <= STOP_ATTACK_FRAMES) {
    enemy->move(0, 0);
    if (enemy->getAttackCooldown() == STOP_ATTACK_FRAMES) {
      enemy->setAttack(true);
      enemy->setAttackDir(p);
    }
    if (enemy->getAttackCooldown() == ATTACK_FRAMES) {
      enemy->addBullet(enemy->getAttackDir());
      _sound_controller->playEnemySmallGunshot();
    }
    if (enemy->getAttackCooldown() <= 0) {
      std::uniform_int_distribution<int> dist(0.0f, 50.0f);
      enemy->setAttackCooldown(dist(_generator) + ATTACK_COOLDOWN);
    }
  } else {
    enemy->setAttackDir(p);
    cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
    diff.normalize();
    diff.add(_direction);
    diff.scale(0.7 * enemy->getSpeed());  // Make speed slower when strafing
    enemy->move(diff.x, diff.y);
  }
}

void ShotgunnerController::changeStateIfApplicable(
    std::shared_ptr<EnemyModel> enemy, float distance) {
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
      // Chance is 1/25 for every tick, +10 to attack frames to ensure does not
      // attack in backwards direction
      if (distance <= MOVE_BACK_RANGE && chance <= 1 &&
          enemy->getAttackCooldown() > STOP_ATTACK_FRAMES + 10) {
        enemy->setCurrentState(EnemyModel::State::MOVING_BACK);
        enemy->_move_back_timer = MOVE_BACK_COOLDOWN;
        enemy->setAttackCooldown(ATTACK_COOLDOWN);
      } else {
        enemy->setCurrentState(EnemyModel::State::ATTACKING);
      }
    }
  } else if (distance <= MIN_DISTANCE) {
    // Only change to chasing if not currently attacking.
    if (enemy->getCurrentState() != EnemyModel::State::ATTACKING ||
        enemy->getAttackCooldown() > STOP_ATTACK_FRAMES) {
      enemy->setCurrentState(EnemyModel::State::CHASING);
      enemy->setAttackCooldown(ATTACK_COOLDOWN);
    }
  } else {
    // Enemy first wanders back to spawn position until it changes to be idle.
    if (enemy->_wander_timer <= 0) {
      enemy->setCurrentState(EnemyModel::State::IDLE);
    }
    if (enemy->getCurrentState() != EnemyModel::State::IDLE &&
        enemy->getCurrentState() != EnemyModel::State::WANDER) {
      enemy->setCurrentState(EnemyModel::State::WANDER);
      enemy->_wander_timer = WANDER_COOLDOWN;  // Spends 6 seconds trying to
                                               // return to original position
    }
    enemy->setAttackCooldown(ATTACK_COOLDOWN);
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

void ShotgunnerController::animate(std::shared_ptr<EnemyModel> enemy) {
  auto node = std::dynamic_pointer_cast<cugl::scene2::SpriteNode>(
      enemy->getNode()->getChildByTag(0));
  auto gun_node = std::dynamic_pointer_cast<cugl::scene2::SpriteNode>(
      enemy->getNode()->getChildByTag(1));
  int fc = enemy->_frame_count;
  if (enemy->getAttackCooldown() <= ATTACK_FRAMES - 5) {
    float frame_angle =
        (enemy->getAttackDir() - enemy->getPosition()).getAngle();
    enemy->setFacingLeft(abs(frame_angle) > M_PI / 2);
    // Here, move back to the original position
    float angle_inc = frame_angle / GUN_MOVE_FRAMES;
    if (enemy->getFacingLeft()) {
      int sign = frame_angle > 0 ? -1 : 1;
      angle_inc = (frame_angle + sign * M_PI) / GUN_MOVE_FRAMES;
    }
    gun_node->setAngle(angle_inc * enemy->getAttackCooldown());
    if (enemy->getAttackCooldown() == 0) {
      gun_node->setAngle(0);
      gun_node->setVisible(false);
    }
  } else if (enemy->getAttackCooldown() >=
                 STOP_ATTACK_FRAMES - GUN_MOVE_FRAMES &&
             enemy->getAttackCooldown() <= STOP_ATTACK_FRAMES) {
    float frame_angle =
        (enemy->getAttackDir() - enemy->getPosition()).getAngle();
    enemy->setFacingLeft(abs(frame_angle) > M_PI / 2);
    // Here, move up to the desired position
    float angle_inc;
    if (enemy->getFacingLeft()) {
      node->setFrame(4);      // Left no gun node
      gun_node->setFrame(5);  // Left gun node
      gun_node->setAnchor(1 - GUN_NODE_X_OFFSET, GUN_NODE_Y_OFFSET);
      int sign = frame_angle > 0 ? -1 : 1;
      angle_inc = (frame_angle + sign * M_PI) / GUN_MOVE_FRAMES;
    } else {
      node->setFrame(1);      // Right no gun node
      gun_node->setFrame(2);  // Right gun node
      gun_node->setAnchor(GUN_NODE_X_OFFSET, GUN_NODE_Y_OFFSET);
      angle_inc = frame_angle / GUN_MOVE_FRAMES;
    }
    gun_node->setVisible(true);
    gun_node->setAngle(angle_inc *
                       (STOP_ATTACK_FRAMES - enemy->getAttackCooldown()));
  } else if (enemy->getAttackCooldown() > ATTACK_FRAMES - 5 &&
             enemy->getAttackCooldown() <
                 STOP_ATTACK_FRAMES - GUN_MOVE_FRAMES) {
  } else if (enemy->getAttackCooldown() > STOP_ATTACK_FRAMES) {
    float length = (enemy->getAttackDir() - enemy->getPosition()).length();
    if (length <= MIN_DISTANCE) {
      gun_node->setVisible(false);

      float direc_angle =
          abs((enemy->getAttackDir() - enemy->getPosition()).getAngle());
      enemy->setFacingLeft(direc_angle > M_PI / 2);

      int run_high_lim = 19;
      int run_low_lim = 10;
      if (enemy->getFacingLeft()) {
        run_high_lim = 29;
        run_low_lim = 20;
      }

      if (node->getFrame() == 0 || node->getFrame() == 3 ||
          node->getFrame() < run_low_lim) {
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
    } else {
      node->setFrame(0);  // Facing right idle
      if (enemy->getFacingLeft()) {
        node->setFrame(3);  // Facing left idle
      }
      enemy->_frame_count = 0;
    }
  }
}

/** Animate the enemy death animation. */
void ShotgunnerController::animateDeath(std::shared_ptr<EnemyModel> enemy) {
  auto node = std::dynamic_pointer_cast<cugl::scene2::SpriteNode>(
      enemy->getNode()->getChildByTag(0));
  if (node->getFrame() < DEATH_RIGHT_LOW_LIM) {
    float direc_angle =
        abs((enemy->getAttackDir() - enemy->getPosition()).getAngle());
    enemy->setFacingLeft(direc_angle > M_PI / 2);
    if (enemy->getFacingLeft()) {
      node->setFrame(DEATH_LEFT_LOW_LIM);
    } else {
      node->setFrame(DEATH_RIGHT_LOW_LIM);
    }
  } else {
    if (enemy->_frame_count >= 2) {
      enemy->_frame_count = 0;
      int next_frame = node->getFrame() + 1;
      if (enemy->getFacingLeft()) {
        if (next_frame == DEATH_LEFT_UP_LIM) {
          next_frame = DEATH_LEFT_UP_LIM - 1;
          enemy->setReadyToDie(true);
        }
      } else {
        if (next_frame == DEATH_RIGHT_UP_LIM) {
          next_frame = DEATH_RIGHT_UP_LIM - 1;
          enemy->setReadyToDie(true);
        }
      }
      node->setFrame(next_frame);
    }
    enemy->_frame_count++;
  }
}
