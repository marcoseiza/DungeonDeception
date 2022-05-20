#include "GruntController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define MOVE_BACK_RANGE 30
#define ATTACK_RANGE 60
#define ATTACK_FRAMES 18
#define STOP_ATTACK_FRAMES 80
#define ATTACK_COOLDOWN 120
#define MOVE_BACK_COOLDOWN 60
#define WANDER_COOLDOWN 500
#define MAX_ATTACK_FRAME 69
#define MIN_ATTACK_FRAME 30

#define STATE_CHANGE_LIM 10

#pragma mark GruntController

void GruntController::clientUpdateAttackPlayer(
    std::shared_ptr<EnemyModel> enemy) {
  if (enemy->didAttack()) {
    // Begin holding.
    enemy->setAttackCooldown(STOP_ATTACK_FRAMES);
    enemy->setAttack(false);
  } else if (enemy->getAttackCooldown() >= -5 &&
             enemy->getAttackCooldown() != ATTACK_COOLDOWN - 1) {
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

void GruntController::attackPlayer(std::shared_ptr<EnemyModel> enemy,
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
    diff.scale(0.7 * enemy->getSpeed());
    enemy->move(diff.x, diff.y);
  }
}

bool GruntController::init(
    std::shared_ptr<cugl::AssetManager> assets,
    std::shared_ptr<cugl::physics2::ObstacleWorld> world,
    std::shared_ptr<cugl::scene2::SceneNode> world_node,
    std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
  _projectile_texture = assets->get<cugl::Texture>("projectile-blue-large");
  EnemyController::init(assets, world, world_node, debug_node);

  return true;
}

void GruntController::changeStateIfApplicable(std::shared_ptr<EnemyModel> enemy,
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
      // Chance is 1/25 for every tick; +10 to attack frames to ensure does not
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

void GruntController::performAction(std::shared_ptr<EnemyModel> enemy,
                                    cugl::Vec2 p) {
  switch (enemy->getCurrentState()) {
    case EnemyModel::State::IDLE: {
      idling(enemy, p);
      break;
    }
    case EnemyModel::State::CHASING: {
      chasePlayer(enemy, p);
      break;
    }
    case EnemyModel::State::ATTACKING: {
      attackPlayer(enemy, p);
      break;
    }
    case EnemyModel::State::STUNNED: {
      stunned(enemy);
      break;
    }
    case EnemyModel::State::WANDER: {
      moveBackToOriginalSpot(enemy);
      break;
    }
    default: {
      avoidPlayer(enemy, p);
      break;
    }
  }
}

void GruntController::animate(std::shared_ptr<EnemyModel> enemy) {
  if (enemy->getHealth() <= 0) {
    animateDeath(enemy);
    return;
  }
  auto node =
      std::dynamic_pointer_cast<cugl::scene2::SpriteNode>(enemy->getNode());
  int fc = enemy->_frame_count;
  if (enemy->getAttackCooldown() <= ATTACK_FRAMES) {
    // Play the next animation frame for the dash attack.
    if (fc >= 4) {
      if (node->getFrame() + 1 < MAX_ATTACK_FRAME &&
          node->getFrame() + 1 > MIN_ATTACK_FRAME) {
        enemy->_frame_count = 0;
        node->setFrame(node->getFrame() + 1);
      } else {
        animateChase(enemy);
      }
    }
    enemy->_frame_count++;
  } else if (enemy->getAttackCooldown() <= STOP_ATTACK_FRAMES) {
    // Hold in the first wind up attack frame depending on direction.
    float frame_angle =
        (enemy->getAttackDir() - enemy->getPosition()).getAngle();
    if (frame_angle <= -M_PI / 2) {
      node->setFrame(60);  // Bottom left
    } else if (frame_angle <= 0) {
      node->setFrame(40);  // Bottom right
    } else if (frame_angle <= M_PI / 2) {
      node->setFrame(30);  // Top right
    } else {
      node->setFrame(50);  // Top left
    }
    enemy->_frame_count = 0;
  } else {
    // Moving animation in all other cases.
    float length = (enemy->getAttackDir() - enemy->getPosition()).length();
    if (length <= MIN_DISTANCE) {
      // Face left or right, depending on direction of player from grunt.
      float direc_angle =
          abs((enemy->getAttackDir() - enemy->getPosition()).getAngle());
      enemy->setFacingLeft(direc_angle > M_PI / 2);
      animateChase(enemy);
    } else {
      // Idle
      node->setFrame(0);
      enemy->_frame_count = 0;
    }
  }
}

void GruntController::animateDeath(std::shared_ptr<EnemyModel> enemy) {}

void GruntController::animateChase(std::shared_ptr<EnemyModel> enemy) {
  auto node =
      std::dynamic_pointer_cast<cugl::scene2::SpriteNode>(enemy->getNode());

  int run_high_lim = 19;
  int run_low_lim = 10;
  if (enemy->getFacingLeft()) {
    run_high_lim = 29;
    run_low_lim = 20;
  }

  if (enemy->_frame_count == 0) {
    node->setFrame(run_low_lim);
  }

  // Play the next animation frame.
  if (enemy->_frame_count >= 4) {
    enemy->_frame_count = 0;
    if (node->getFrame() >= run_high_lim || node->getFrame() < run_low_lim) {
      node->setFrame(run_low_lim);
    } else {
      node->setFrame(node->getFrame() + 1);
    }
  }
  enemy->_frame_count++;
}
