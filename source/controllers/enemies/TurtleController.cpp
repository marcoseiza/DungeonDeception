#include "TurtleController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 200
#define TANK_RANGE 60

#define ATTACK_FRAME_POS 35
#define ATTACK_COOLDOWN 60

#define TURTLE_OPENED 16
#define CLOSED 23
#define ATTACK_RIGHT 32
#define ATTACK_UP 44
#define ATTACK_DOWN 36
#define ATTACK_LEFT 40
#define ATTACK_BOTTOM_LEFT 38
#define ATTACK_BOTTOM_RIGHT 34
#define ATTACK_TOP_RIGHT 46
#define ATTACK_TOP_LEFT 42
#define ATTACK_HALF 41
#define ATTACK_UP_LIM 48
#define DEATH_RIGHT_LOW_LIM 48
#define DEATH_RIGHT_UP_LIM 70
#define DEATH_LEFT_LOW_LIM 80
#define DEATH_LEFT_UP_LIM 102

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

void TurtleController::clientUpdateAttackPlayer(
    std::shared_ptr<EnemyModel> enemy) {
  if (enemy->didAttack()) {
    // Begin attack cooldown.
    enemy->setAttackCooldown(ATTACK_FRAME_POS - 1);
    enemy->setAttack(false);
  } else if (enemy->getAttackCooldown() >= 0 &&
             enemy->getAttackCooldown() != ATTACK_COOLDOWN - 1) {
    // Whether the enemy is attacking
    if (enemy->getAttackCooldown() == 0) {
      float frame_angle =
          (enemy->getAttackDir() - enemy->getPosition()).getAngle();
      // Rotate this vector accordingly
      cugl::Vec2 att_vec = enemy->getPosition() + cugl::Vec2(1, 0);
      if (frame_angle < -(7.0 / 8.0) * M_PI) {
        enemy->addBullet(att_vec.rotate(M_PI, enemy->getPosition()));
      } else if (frame_angle < -(5.0 / 8.0) * M_PI) {
        enemy->addBullet(
            att_vec.rotate(-(3.0 / 4.0) * M_PI, enemy->getPosition()));
      } else if (frame_angle < -(3.0 / 8.0) * M_PI) {
        enemy->addBullet(
            att_vec.rotate(-(1.0 / 2.0) * M_PI, enemy->getPosition()));
      } else if (frame_angle < -(1.0 / 8.0) * M_PI) {
        enemy->addBullet(
            att_vec.rotate(-(1.0 / 4.0) * M_PI, enemy->getPosition()));
      } else if (frame_angle < (1.0 / 8.0) * M_PI) {
        enemy->addBullet(att_vec.rotate(0, enemy->getPosition()));
      } else if (frame_angle < (3.0 / 8.0) * M_PI) {
        enemy->addBullet(
            att_vec.rotate((1.0 / 4.0) * M_PI, enemy->getPosition()));
      } else if (frame_angle < (5.0 / 8.0) * M_PI) {
        enemy->addBullet(
            att_vec.rotate((1.0 / 2.0) * M_PI, enemy->getPosition()));
      } else if (frame_angle < (7.0 / 8.0) * M_PI) {
        enemy->addBullet(
            att_vec.rotate((3.0 / 4.0) * M_PI, enemy->getPosition()));
      } else {
        enemy->addBullet(att_vec.rotate(M_PI, enemy->getPosition()));
      }
      _sound_controller->playEnemyLargeGunshot();
      enemy->setAttackCooldown(ATTACK_COOLDOWN);
    }
    enemy->reduceAttackCooldown(1);
  }
}

void TurtleController::attackPlayer(std::shared_ptr<EnemyModel> enemy,
                                    const cugl::Vec2 p) {
  if (enemy->getAttackCooldown() <= 0) {
    float frame_angle =
        (enemy->getAttackDir() - enemy->getPosition()).getAngle();
    // Rotate this vector accordingly
    cugl::Vec2 att_vec = enemy->getPosition() + cugl::Vec2(1, 0);
    if (frame_angle < -(7.0 / 8.0) * M_PI) {
      enemy->addBullet(att_vec.rotate(M_PI, enemy->getPosition()));
    } else if (frame_angle < -(5.0 / 8.0) * M_PI) {
      enemy->addBullet(
          att_vec.rotate(-(3.0 / 4.0) * M_PI, enemy->getPosition()));
    } else if (frame_angle < -(3.0 / 8.0) * M_PI) {
      enemy->addBullet(
          att_vec.rotate(-(1.0 / 2.0) * M_PI, enemy->getPosition()));
    } else if (frame_angle < -(1.0 / 8.0) * M_PI) {
      enemy->addBullet(
          att_vec.rotate(-(1.0 / 4.0) * M_PI, enemy->getPosition()));
    } else if (frame_angle < (1.0 / 8.0) * M_PI) {
      enemy->addBullet(att_vec.rotate(0, enemy->getPosition()));
    } else if (frame_angle < (3.0 / 8.0) * M_PI) {
      enemy->addBullet(
          att_vec.rotate((1.0 / 4.0) * M_PI, enemy->getPosition()));
    } else if (frame_angle < (5.0 / 8.0) * M_PI) {
      enemy->addBullet(
          att_vec.rotate((1.0 / 2.0) * M_PI, enemy->getPosition()));
    } else if (frame_angle < (7.0 / 8.0) * M_PI) {
      enemy->addBullet(
          att_vec.rotate((3.0 / 4.0) * M_PI, enemy->getPosition()));
    } else {
      enemy->addBullet(att_vec.rotate(M_PI, enemy->getPosition()));
    }

    _sound_controller->playEnemyLargeGunshot();
    enemy->setAttackCooldown(ATTACK_COOLDOWN);
  } else if (enemy->getAttackCooldown() == ATTACK_FRAME_POS) {
    enemy->setAttackDir(p);
    enemy->setAttack(true);
  }
  if (enemy->getAttackCooldown() > ATTACK_FRAME_POS) {
    enemy->setAttackDir(p);
  }
  enemy->move(0, 0);
}

void TurtleController::tank(std::shared_ptr<EnemyModel> enemy,
                            const cugl::Vec2 p) {
  enemy->move(0, 0);
  enemy->setAttackDir(p);
}

void TurtleController::changeStateIfApplicable(
    std::shared_ptr<EnemyModel> enemy, float distance) {
  if (distance <= TANK_RANGE) {
    if (enemy->getCurrentState() != EnemyModel::State::ATTACKING ||
        enemy->getAttackCooldown() > ATTACK_FRAME_POS) {
      enemy->setCurrentState(EnemyModel::State::TANKING);
    }
    enemy->setAttackCooldown(ATTACK_COOLDOWN);
  } else if (distance <= ATTACK_RANGE) {
    enemy->setCurrentState(EnemyModel::State::ATTACKING);
  } else {
    enemy->setCurrentState(EnemyModel::State::IDLE);
    enemy->setAttackCooldown(ATTACK_COOLDOWN);
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
      tank(enemy, p);
      break;
    }
    default: {
      idling(enemy, p);
    }
  }
}

void TurtleController::animate(std::shared_ptr<EnemyModel> enemy) {
  auto node =
      std::dynamic_pointer_cast<cugl::scene2::SpriteNode>(enemy->getNode());
  int fc = enemy->_frame_count;
  float length = (enemy->getAttackDir() - enemy->getPosition()).length();
  if (length <= TANK_RANGE) {
    animateClose(enemy);
  } else {
    if (node->getFrame() > CLOSED || node->getFrame() < TURTLE_OPENED) {
      // Already opened.
      if (enemy->getAttackCooldown() == ATTACK_FRAME_POS - 1) {
        float frame_angle =
            (enemy->getAttackDir() - enemy->getPosition()).getAngle();
        if (frame_angle < -M_PI * (7.0 / 8.0)) {
          enemy->_goal_frame = ATTACK_LEFT;
        } else if (frame_angle < -(5.0 / 8.0) * M_PI) {
          enemy->_goal_frame = ATTACK_BOTTOM_LEFT;
        } else if (frame_angle < -(3.0 / 8.0) * M_PI) {
          enemy->_goal_frame = ATTACK_DOWN;
        } else if (frame_angle < -(1.0 / 8.0) * M_PI) {
          enemy->_goal_frame = ATTACK_BOTTOM_RIGHT;
        } else if (frame_angle < (1.0 / 8.0) * M_PI) {
          enemy->_goal_frame = ATTACK_RIGHT;
          if (node->getFrame() >= ATTACK_HALF) {  // Move the other way
            enemy->_goal_frame = ATTACK_UP_LIM;
          }
        } else if (frame_angle < (3.0 / 8.0) * M_PI) {
          enemy->_goal_frame = ATTACK_TOP_RIGHT;
        } else if (frame_angle < (5.0 / 8.0) * M_PI) {
          enemy->_goal_frame = ATTACK_UP;
        } else if (frame_angle < (7.0 / 8.0) * M_PI) {
          enemy->_goal_frame = ATTACK_TOP_LEFT;
        } else {
          enemy->_goal_frame = ATTACK_LEFT;
        }
        if (node->getFrame() == 0 || node->getFrame() == 1) {
          node->setFrame(ATTACK_RIGHT);
        }
      } else if (enemy->getAttackCooldown() < ATTACK_FRAME_POS) {
        if (fc >= 2 && enemy->_goal_frame != node->getFrame()) {
          if (node->getFrame() == ATTACK_RIGHT &&
              enemy->_goal_frame >= ATTACK_HALF) {
            node->setFrame(ATTACK_UP_LIM - 1);
          } else if (enemy->_goal_frame < node->getFrame()) {
            node->setFrame(node->getFrame() - 1);
          } else {
            if (node->getFrame() + 1 == ATTACK_UP_LIM) {
              node->setFrame(ATTACK_RIGHT);
              enemy->_goal_frame = ATTACK_RIGHT;
            } else {
              node->setFrame(node->getFrame() + 1);
            }
          }
          enemy->_frame_count = 0;
        }
        enemy->_frame_count++;
      }
    } else {
      animateOpen(enemy);
    }
  }
}

/** Animate the enemy death animation. */
void TurtleController::animateDeath(std::shared_ptr<EnemyModel> enemy) {
  auto node =
      std::dynamic_pointer_cast<cugl::scene2::SpriteNode>(enemy->getNode());
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
    if (enemy->_frame_count >= 4) {
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

void TurtleController::animateClose(std::shared_ptr<EnemyModel> enemy) {
  auto node =
      std::dynamic_pointer_cast<cugl::scene2::SpriteNode>(enemy->getNode());
  int fc = enemy->_frame_count;

  if (fc >= 2) {
    if (node->getFrame() == CLOSED) {
    } else {
      if (node->getFrame() > ATTACK_RIGHT && node->getFrame() < ATTACK_HALF) {
        node->setFrame(node->getFrame() - 1);
        if (node->getFrame() == ATTACK_RIGHT) {
          node->setFrame(TURTLE_OPENED);
        }
      } else {
        if (node->getFrame() + 1 == ATTACK_UP_LIM) {
          node->setFrame(TURTLE_OPENED);
        } else {
          node->setFrame(node->getFrame() + 1);
        }
      }
    }
    enemy->_frame_count = 0;
  }
  enemy->_frame_count++;
}

void TurtleController::animateOpen(std::shared_ptr<EnemyModel> enemy) {
  auto node =
      std::dynamic_pointer_cast<cugl::scene2::SpriteNode>(enemy->getNode());
  int fc = enemy->_frame_count;

  if (node->getFrame() > CLOSED || node->getFrame() < TURTLE_OPENED) {
  } else if (fc >= 2) {
    if (node->getFrame() == TURTLE_OPENED) {
      node->setFrame(ATTACK_RIGHT);
    } else {
      node->setFrame(node->getFrame() - 1);
    }
    enemy->_frame_count = 0;
  }
  enemy->_frame_count++;
}
