#include "Sword.h"

// Offset from the player
#define OFFSET_X_HOR 30
#define OFFSET_Y_HOR 5

#define OFFSET_X_VERT 0
#define OFFSET_Y_VERT 30

#define SIZE 25

bool Sword::init(const cugl::Vec2 pos) {
  CapsuleObstacle::init(cugl::Vec2(pos.x + OFFSET_X_HOR, pos.y + OFFSET_Y_HOR),
                        cugl::Size(SIZE, SIZE));

  setBodyType(b2_kinematicBody);
  setDensity(0.01f);
  setFriction(0.0f);
  setRestitution(0.01f);
  setFixedRotation(true);
  setSensor(true);

  return true;
}

void Sword::moveSword(const cugl::Vec2 pos, const cugl::Vec2 vel, int moveDir) {
  if (moveDir == 0) {
    setPosition(pos.x + OFFSET_X_HOR * -1, pos.y + OFFSET_Y_HOR);
  } else if (moveDir == 1) {
    setPosition(pos.x + OFFSET_X_VERT, pos.y + OFFSET_Y_VERT * -1);
  } else if (moveDir == 2) {
    setPosition(pos.x + OFFSET_X_HOR, pos.y + OFFSET_Y_HOR);
  } else {
    setPosition(pos.x + OFFSET_X_VERT, pos.y + OFFSET_Y_VERT);
  }

  setVX(1000 * vel.x);
  setVY(1000 * vel.y);
}
