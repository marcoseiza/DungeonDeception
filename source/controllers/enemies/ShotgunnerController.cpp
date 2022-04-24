#include "ShotgunnerController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 150
#define ATTACK_FRAMES 20
#define STOP_ATTACK_FRAMES 40
#define ATTACK_COOLDOWN 100

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
    }
    if (enemy->getAttackCooldown() <= 0) {
      std::uniform_int_distribution<int> dist(0.0f, 50.0f);
      enemy->setAttackCooldown(dist(_generator) + ATTACK_COOLDOWN);
    }
  } else {
    cugl::Vec2 diff = cugl::Vec2(enemy->getVX(), enemy->getVY());
    diff.normalize();
    diff.add(_direction);
    diff.scale(0.6 * enemy->getSpeed()); // Make speed slower when strafing
    enemy->move(diff.x, diff.y);
  }
}

void ShotgunnerController::changeStateIfApplicable(
    std::shared_ptr<EnemyModel> enemy, float distance) {
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
    default: {
      idling(enemy);
    }
  }
}

void ShotgunnerController::animate(std::shared_ptr<EnemyModel> enemy) {
  auto node = dynamic_cast<cugl::scene2::SpriteNode*>(enemy->getNode()->getChildByTag(0).get());
  auto gun_node = enemy->getNode()->getChildByTag(1);
  int fc = enemy->_frame_count;
  switch (enemy->getCurrentState()) {
    case EnemyModel::State::ATTACKING: {
      if (enemy->getAttackCooldown() <= ATTACK_FRAMES - 5) {
        float angle_inc = cugl::Vec2::angle(cugl::Vec2(1, 0), enemy->_attack_dir - enemy->getPosition()) / 15;
        gun_node->setAngle(angle_inc * enemy->getAttackCooldown());
        if (enemy->getAttackCooldown() == 0) {
          gun_node->setAngle(0);
          gun_node->setVisible(false);
        }
        break;
      } else if (enemy->getAttackCooldown() >= ATTACK_FRAMES + 5 && enemy->getAttackCooldown() <= STOP_ATTACK_FRAMES) {
        // Here, move up to the desired position
        node->setFrame(1);
        gun_node->setVisible(true);
        float angle_inc = cugl::Vec2::angle(cugl::Vec2(1, 0), enemy->_attack_dir - enemy->getPosition()) / 15;
        gun_node->setAngle(angle_inc * (STOP_ATTACK_FRAMES - enemy->getAttackCooldown()));
        break;
      } else if (enemy->getAttackCooldown() < ATTACK_FRAMES + 5 && enemy->getAttackCooldown() > ATTACK_FRAMES - 5) {
        break;
      }
    }
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
