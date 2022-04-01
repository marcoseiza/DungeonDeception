#ifndef MODELS_SWORD_H_
#define MODELS_SWORD_H_

#include <cugl/cugl.h>
#include <stdio.h>

class Sword : public cugl::physics2::CapsuleObstacle {
 private:
 public:
#pragma mark Constructors
  /**
   * Creates the sword.
   */
  Sword(void) : CapsuleObstacle() {}

  /**
   * Disposes the sword.
   */
  ~Sword() {}

  /**
   * Initializes the sword with the given position and size.
   * @param  pos      Initial position in world coordinates.
   * @param  size       The dimensions of the box.
   *
   * @return  true if the obstacle is initialized properly, false otherwise.
   */
  virtual bool init(const cugl::Vec2 pos);

#pragma mark Static Constructors
  /**
   * Returns a new capsule object at the given point.
   *
   * @param pos   Initial position in world coordinates.
   * @param size The size of the object.
   *
   * @return a new capsule object at the given point with no size.
   */
  static std::shared_ptr<Sword> alloc(const cugl::Vec2 pos) {
    std::shared_ptr<Sword> result = std::make_shared<Sword>();
    return (result->init(pos) ? result : nullptr);
  }

#pragma mark Properties

  /**
   * Resets the sword position with the corresponding offset.
   *
   * @param pos the position to move the sword to.
   * @param vel the velocity to move the sword by.
   * @param moveDir if sword should be on left or right or up or down.
   */
  void moveSword(cugl::Vec2 pos, cugl::Vec2 vel, int moveDir);
};

#endif /* Sword_h */
