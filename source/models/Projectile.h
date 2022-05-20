#ifndef MODELS_PROJECTILE_H_
#define MODELS_PROJECTILE_H_

#include <cugl/cugl.h>
#include <stdio.h>

#define MAX_LIFETIME 300
#define MAX_LIVE_FRAMES 42  // Must match player slash animation frames

class Projectile : public cugl::physics2::CapsuleObstacle {
 private:
  /** Number of frames until dead */
  int _live_frames;

  /** Whether need to add to the world or not. */
  bool _in_world;

  /** Whether it should die or not. */
  bool _is_dead;
  
  /** How long the projectile has been alive */
  int _lifetime;
  

  /** The scene graph node for the projectile. */
  std::shared_ptr<cugl::scene2::SpriteNode> _projectile_node;

 public:
#pragma mark Constructors
  /**
   * Creates the sword.
   */
  Projectile(void) : CapsuleObstacle() {}

  /**
   * Disposes the sword.
   */
  ~Projectile() { dispose(); }

  /**
   * Initializes the sword with the given position and size.
   * @param  pos      Initial position in world coordinates.
   * @param  size       The dimensions of the box.
   *
   * @return  true if the obstacle is initialized properly, false otherwise.
   */
  bool init(const cugl::Vec2 pos, const cugl::Vec2 v, int speed, int live_frames);

  /**
   * Disposes the projectile.
   */
  void dispose() { _projectile_node = nullptr; }

#pragma mark Static Constructors
  /**
   * Returns a new capsule object at the given point.
   *
   * @param pos   Initial position in world coordinates.
   * @param size The size of the object.
   *
   * @return a new capsule object at the given point with no size.
   */
  static std::shared_ptr<Projectile> alloc(const cugl::Vec2 pos,
                                           const cugl::Vec2 v, int speed = 300, int live_frames = MAX_LIVE_FRAMES) {
    std::shared_ptr<Projectile> result = std::make_shared<Projectile>();
    return (result->init(pos, v, speed, live_frames) ? result : nullptr);
  }

#pragma mark Properties
  /**
   * Decrement the number of frames left for this projectile.
   *
   * @param frames Number to decrement frames by.
   */
  void decrementFrame(int frames) { _live_frames -= frames; }

  /**
   * Get the number of frames.
   *
   * @return Number of frames left
   */
  int getFrames() { return _live_frames; }

  /**
   * Set the number of frames.
   *
   * @param frames Set number frames to frames
   */
  void setFrames(int frames) { _live_frames = frames; }

  /**
   * Sets whether the projectile has been added to the obstacle world.
   *
   * @param in_world if the projectile should have been added to the world.
   */
  void setInWorld(bool in_world) { _in_world = in_world; }

  /**
   * If the bullet is in the world.
   *
   * @return if the bullet is in the world.
   */
  bool isInWorld() { return _in_world; }

  /**
   * Sets whether the projectile has been hit.
   *
   * @param is_dead if the projectile should be dead.
   */
  void setDead(bool is_dead) { _is_dead = is_dead; }

  /**
   * If the bullet is dead.
   *
   * @return if the bullet is dead.
   */
  bool isDead() { return _is_dead; }
  
  /**
   * Returns how long the projectile has been alive.
   *
   * @return how long the projectile has been alive.
   */
  int getLifetime() { return _lifetime; }
  
  /**
   * Sets the lifetime of the projectile. Usually used to update lifetime every tick.
   */
  void setLifetime(int lifetime) { _lifetime = lifetime; }
  
  /**
   * Returns whether a projectile should die because it has exceeded its max lifespan.
   *
   * @return true if a projectile should die of old age.
   */
  bool shouldExpire() { return _lifetime >= MAX_LIFETIME; }

#pragma mark Graphics
  /**
   * Sets the scene graph node representing this projectile.
   *
   * @param node  The scene graph node representing this projectile.
   */
  void setNode(const std::shared_ptr<cugl::scene2::SpriteNode>& node) {
    _projectile_node = node;
  }

  /**
   * Gets the projectile scene graph node.
   *
   * @return node the node that has been set.
   */
  std::shared_ptr<cugl::scene2::SpriteNode>& getNode() {
    return _projectile_node;
  }
};

#endif /* Projectile.h */
