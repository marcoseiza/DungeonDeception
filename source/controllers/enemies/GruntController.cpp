#include "GruntController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 60
#define ATTACK_FRAMES 18
#define STOP_ATTACK_FRAMES 50
#define ATTACK_COOLDOWN 100

#define STATE_CHANGE_LIM 10

#pragma mark GruntController

void GruntController::attackPlayer(std::shared_ptr<EnemyModel> enemy,
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
      dir.scale(3.5);
      enemy->move(dir.x, dir.y);
    }
  } else if (enemy->getAttackCooldown() <= STOP_ATTACK_FRAMES) {
    enemy->move(0, 0);
    if (enemy->getAttackCooldown() == STOP_ATTACK_FRAMES) {
      enemy->setAttack(true);
      enemy->_attack_dir = p - enemy->getPosition();
      enemy->_attack_dir.normalize();
    }
  } else {
    enemy->_attack_dir = p - enemy->getPosition();
    cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
    diff.normalize();
    diff.add(_direction);
    diff.scale(0.6 * enemy->getSpeed());
    enemy->move(diff.x, diff.y);
  }
  
//  if (enemy->getAttackCooldown() <= 0) {
//    // Just finished attack, reset attack cooldown.
//    std::uniform_int_distribution<int> dist(0.0f, 50.0f);
//    enemy->setAttackCooldown(dist(_generator) + ATTACK_COOLDOWN);
//    enemy->resetSensors();
//  } else if (enemy->getAttackCooldown() < ATTACK_FRAMES) {
//    // Currently attacking player.
//    cugl::Vec2 dir = enemy->_attack_dir;
//    dir.scale(3.5);
//    enemy->move(dir.x, dir.y);
//  } else if (enemy->getAttackCooldown() == ATTACK_FRAMES) {
//    // Begin attack.
//    enemy->setSensor(true);
//    _sound_controller->playEnemySwing();
//  } else if (enemy->getAttackCooldown() < STOP_ATTACK_FRAMES) {
//    // Stops in place to wind up attack.
//    enemy->move(0, 0);
//    if (enemy->getAttackCooldown() == STOP_ATTACK_FRAMES) {
//      // Determine attack position.
//      enemy->setAttack(true);
//      enemy->_attack_dir = p - enemy->getPosition();
//      enemy->_attack_dir.normalize();
//    }
//  } else {
//    // Circle the player.
//    enemy->_attack_dir = p;
//    cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
//    diff.normalize();
//    diff.add(_direction);
//    diff.scale(0.6 * enemy->getSpeed());
//    enemy->move(diff.x, diff.y);
//  }
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
    default: {
      avoidPlayer(enemy, p);
      break;
    }
  }
}

void GruntController::animate(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p) {
  auto node =
      std::dynamic_pointer_cast<cugl::scene2::SpriteNode>(enemy->getNode());
  int fc = enemy->_frame_count;
  switch (enemy->getCurrentState()) {
    case EnemyModel::State::ATTACKING: {
      if (enemy->getAttackCooldown() <= ATTACK_FRAMES + 8) {
        // Play the next animation frame.
        if (fc >= 3) {
          enemy->_frame_count = 0;
          node->setFrame(node->getFrame() + 1);
        }
        enemy->_frame_count++;
      } else if (enemy->getAttackCooldown() <= STOP_ATTACK_FRAMES) {
        // Depending on direction, set the frame.
        float frame_angle = enemy->_attack_dir.getAngle();
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
        // Look at the direction of the player when circling
        float direc_angle =
            abs(cugl::Vec2(p - enemy->getPosition()).getAngle());
        enemy->setFacingLeft(direc_angle > M_PI / 2);
        animateChase(enemy);
      }
      break;
    }
    case EnemyModel::State::CHASING: {
      if (enemy->getVX() != 0) {
        if (enemy->getVX() < 0 != enemy->getFacingLeft()) {
          enemy->_frame_count = 0;
          enemy->setFacingLeft(enemy->getVX() < 0);
        }
      }
      animateChase(enemy);
      break;
    }
    default: {
      node->setFrame(0);
      enemy->_frame_count = 0;
      break;
    }
  }
}

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
    if (node->getFrame() >= run_high_lim) {
      node->setFrame(run_low_lim);
    } else {
      node->setFrame(node->getFrame() + 1);
    }
  }
  enemy->_frame_count++;
}
