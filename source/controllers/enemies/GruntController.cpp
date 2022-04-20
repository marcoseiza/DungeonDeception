#include "GruntController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 60
#define ATTACK_FRAMES 18
#define STOP_ATTACK_FRAMES 50
#define ATTACK_COOLDOWN 155

#define STATE_CHANGE_LIM 10

#pragma mark GruntController

void GruntController::attackPlayer(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p) {
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
      dir.scale(3.5);
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
      idling(enemy);
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

void GruntController::animate(std::shared_ptr<EnemyModel> enemy) {
  auto node = dynamic_cast<cugl::scene2::SpriteNode*>(enemy->getNode().get());
  int fc = enemy->_frame_count;
  switch (enemy->getCurrentState()) {
    case EnemyModel::State::ATTACKING: {
      if (enemy->getAttackCooldown() <= ATTACK_FRAMES + 8) {
        int attack_high_lim = 29;
        int attack_low_lim = 20;

        // Play the next animation frame.
        if (fc >= 3) {
          enemy->_frame_count = 0;
          if (node->getFrame() >= attack_high_lim) {
            node->setFrame(attack_low_lim);
          } else {
            node->setFrame(node->getFrame() + 1);
          }
        }
        enemy->_frame_count++;
        break;
      } else if (enemy->getAttackCooldown() <= STOP_ATTACK_FRAMES) {
        node->setFrame(20);
        break;
      }
    }
    case EnemyModel::State::CHASING: {
      int run_high_lim = 19;
      int run_low_lim = 10;

      if (fc == 0) {
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
