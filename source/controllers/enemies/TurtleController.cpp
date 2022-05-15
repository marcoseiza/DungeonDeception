#include "TurtleController.h"

#define MIN_DISTANCE 300
#define HEALTH_LIM 25
#define ATTACK_RANGE 200
#define TANK_RANGE 60

#define ATTACK_FRAME_POS 25
#define ATTACK_COOLDOWN 60

#define TURTLE_OPENED 16
#define CLOSED 23
#define ATTACK_RIGHT 32
#define ATTACK_UP 44
#define ATTACK_DOWN 36
#define ATTACK_LEFT 40
#define ATTACK_HALF 41
#define ATTACK_UP_LIM 48

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

void TurtleController::clientUpdateAttackPlayer(std::shared_ptr<EnemyModel> enemy) {
  if (enemy->didAttack()) {
    // Begin attack cooldown.
    enemy->setAttackCooldown(ATTACK_FRAME_POS);
    enemy->setAttack(false);
  } else if (enemy->getAttackCooldown() >= 0 && enemy->getAttackCooldown() != ATTACK_COOLDOWN - 1) {
    // Whether the enemy is attacking
    if (enemy->getAttackCooldown() == 0) {
      // Do attack done.
      cugl::Vec2 e = enemy->getPosition();
      cugl::Vec2 p1 = enemy->getAttackDir();

      // Attack in closest cardinal direction
      if (abs(p1.x - e.x) > abs(p1.y - e.y)) {
        int add = (p1.x - e.x > 0) ? 1 : -1;
        enemy->addBullet(cugl::Vec2(e.x + add, e.y));
      } else {
        int add = (p1.y - e.y > 0) ? 1 : -1;
        enemy->addBullet(cugl::Vec2(e.x, e.y + add));
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
    cugl::Vec2 e = enemy->getPosition();
    cugl::Vec2 p1 = enemy->getAttackDir();

    // Attack in closest cardinal direction
    if (abs(p1.x - e.x) > abs(p1.y - e.y)) {
      int add = (p1.x - e.x > 0) ? 1 : -1;
      enemy->addBullet(cugl::Vec2(e.x + add, e.y));
    } else {
      int add = (p1.y - e.y > 0) ? 1 : -1;
      enemy->addBullet(cugl::Vec2(e.x, e.y + add));
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

void TurtleController::tank(std::shared_ptr<EnemyModel> enemy, const cugl::Vec2 p) {
  enemy->move(0, 0);
  enemy->setAttackDir(p);
}

void TurtleController::changeStateIfApplicable(
    std::shared_ptr<EnemyModel> enemy, float distance) {
  if (distance <= TANK_RANGE) {
    if (enemy->getCurrentState() != EnemyModel::State::TANKING) {
      enemy->setCurrentState(EnemyModel::State::TANKING);
    }
    enemy->setAttackCooldown(ATTACK_COOLDOWN);
  } else if (distance <= ATTACK_RANGE) {
    if (enemy->getCurrentState() != EnemyModel::State::ATTACKING) {
      enemy->setCurrentState(EnemyModel::State::ATTACKING);
    }
  } else {
    if (enemy->getCurrentState() != EnemyModel::State::IDLE) {
      enemy->setCurrentState(EnemyModel::State::IDLE);
    }
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
  if (length <= TANK_RANGE || length >= ATTACK_RANGE) {
    animateClose(enemy);
  } else {
      if (node->getFrame() > CLOSED || node->getFrame() < TURTLE_OPENED) {
        // Already opened.
        if (enemy->getAttackCooldown() == ATTACK_FRAME_POS - 1) {
          cugl::Vec2 e = enemy->getPosition();
          cugl::Vec2 p1 = enemy->getAttackDir();
          if (abs(p1.x - e.x) > abs(p1.y - e.y)) {
            if (p1.x - e.x > 0) {
              enemy->_goal_frame = ATTACK_RIGHT;      // Face right
              if (node->getFrame() >= ATTACK_HALF) {  // Move the other way
                enemy->_goal_frame = ATTACK_UP_LIM;
              }
            } else {
              enemy->_goal_frame = ATTACK_LEFT;  // Face left
            }
          } else {
            if (p1.y - e.y > 0) {
              enemy->_goal_frame = ATTACK_UP;  // Face up
            } else {
              enemy->_goal_frame = ATTACK_DOWN;  // Face down
            }
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
