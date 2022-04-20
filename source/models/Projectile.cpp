#include "Projectile.h"

#include "../controllers/CollisionFiltering.h"

#define MAX_LIVE_FRAMES 42  // Must match player slash animation frames

#pragma mark Init
bool Projectile::init(const cugl::Vec2 pos, const cugl::Vec2 v) {
  CapsuleObstacle::init(pos, cugl::Size(5, 5));
  cugl::Vec2 v2 = cugl::Vec2(v * 300);
  setVX(v2.x);
  setVY(v2.y);
  setSensor(true);
  setDensity(0.01f);
  setFriction(0.0f);
  setRestitution(0.01f);
  setFixedRotation(true);

  setName("projectile");

  _fixture.filter.categoryBits = CATEGORY_PROJECTILE;
  _fixture.filter.maskBits = MASK_PROJECTILE;

  _live_frames = MAX_LIVE_FRAMES;
  _in_world = false;

  return true;
}
