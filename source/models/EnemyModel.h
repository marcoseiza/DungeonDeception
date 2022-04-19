#pragma once

#ifndef MODELS_ENEMYMODEL_H
#define MODELS_ENEMYMODEL_H

#include <cugl/cugl.h>
#include <stdio.h>

#include "Projectile.h"

class EnemyModel : public cugl::physics2::CapsuleObstacle {
 public:
  /** Enum for the enemy's state (for animation). */
  enum State {
    /** The enemy is not moving. */
    IDLE,
    /** The enemy is chasing the player. */
    CHASING,
    /** The enemy is attacking the player. */
    ATTACKING,
    /** The enemy is avoiding the player. */
    AVOIDING,
    /** The enemy is tanking the player. */
    TANKING,
    /** The enemy is skirting the player. */
    SKIRTING
  };

  /** Enum for which enemy this is. */
  enum EnemyType {
    /** The EnemyModel enemy type. */
    GRUNT,
    /** The shotgunner enemy type. */
    SHOTGUNNER,
    /** The tank enemy type. */
    TANK,
    /** The turtle enemy type. */
    TURTLE
  };

 private:
  /** The current state of the enemy. */
  State _current_state;

  /** The enemy type of this enemy. */
  EnemyType _enemy_type;

  /** Enemy unique id. */
  int _id;

  /** Enemy health. */
  int _health;

  /** Enemy speed. */
  float _speed;

  /** The scene graph node for the enemy. */
  std::shared_ptr<cugl::scene2::SpriteNode> _enemy_node;

  /** Represents the hit area for the enemy. */
  b2Fixture* _hitbox_sensor;
  /** Keeps an instance of the name alive for collision detection. */
  std::shared_ptr<std::string> _hitbox_sensor_name;
  /** The node for debugging the hitbox sensor */
  std::shared_ptr<cugl::scene2::WireNode> _hitbox_sensor_node;

  /** Represents the hit area for the enemy. */
  b2Fixture* _damage_sensor;
  /** Keeps an instance of the name alive for collision detection. */
  std::shared_ptr<std::string> _damage_sensor_name;
  /** The node for debugging the damage sensor */
  std::shared_ptr<cugl::scene2::WireNode> _damage_sensor_node;

  /** The list of projectiles that have been shot by the enemy. */
  std::unordered_set<std::shared_ptr<Projectile>> _projectiles;

  /** Force to be applied to the enemy. */
  cugl::Vec2 _force;

  /** enemy direction. */
  bool _facing_left;

  /** Damage frame count to turn red. */
  int _damage_count;

  /** Attack cooldown. */
  int _attack_cooldown;

  /** Represents the offset between the center of the player and the center of
   * the capsule obstacle. */
  cugl::Vec2 _offset_from_center;

  /** The position of the room this enemy is in, used for drawing. */
  cugl::Vec2 _room_pos;

  /** Promise to change the physics state during the update phase. */
  bool _promise_to_change_physics;
  /** If the promise to change physics state should enable the body or
   * disable it */
  bool _promise_to_enable;

 public:
  
  /** The set of polygon nodes corresponding to the weights for the direction of the enemy. */
  std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> _polys;
  
  /** Whether the enemy is moving in a CW motion (when attacking). */
  bool _move_CW;
  
  /** Timer for switching from attack to chase. */
  int _atc_timer;
  
  /** Timer for switching from chase to attack. */
  int _cta_timer;
  
  /** When attacking, direction to attack in. */
  cugl::Vec2 _attack_dir;
  
  /** The count for switching to the next frame. */
  int _frame_count;
  
#pragma mark Constructors
  /**
   * Creates a enemy with the given position and data.
   *
   * @param pos The enemy position.
   * @param data The data defining the enemy.
   */
  EnemyModel(void)
      : CapsuleObstacle(),
        _hitbox_sensor(nullptr),
        _hitbox_sensor_name(nullptr),
        _damage_sensor(nullptr),
        _damage_sensor_name(nullptr) {}

  /**
   * Disposes the grunt.
   */
  ~EnemyModel() { dispose(); }

  /**
   * Disposes the grunt.
   */
  void dispose() {
    _enemy_node = nullptr;
    _hitbox_sensor = nullptr;
    _damage_sensor = nullptr;
    _projectiles.clear();
  }

  /**
   * Initializes a new grunt with the given position and size.
   *
   * @param  pos      Initial position in world coordinates.
   * @param  size       The dimensions of the box.
   *
   * @return  true if the obstacle is initialized properly, false otherwise.
   */
  bool init(const cugl::Vec2 pos, string name, string type);

#pragma mark Static Constructors
  /**
   * Returns a new capsule object at the given point with no size.
   *
   * @param pos   Initial position in world coordinates.
   *
   * @return a new capsule object at the given point with no size.
   */
  static std::shared_ptr<EnemyModel> alloc(const cugl::Vec2 pos, string name,
                                           string type) {
    std::shared_ptr<EnemyModel> result = std::make_shared<EnemyModel>();
    return (result->init(pos, name, type) ? result : nullptr);
  }

#pragma mark Properties

  /**
   * Returns the unique id of the enemy.
   *
   * @return the enemy id.
   */
  int getEnemyId() const { return _id; }

  /**
   * Sets the current enemy's unique id.
   *
   * @param id The current enemy's id.
   */
  void setEnemyId(int id) { _id = id; }

  /**
   * Returns the current health of the enemy.
   *
   * @return the current health.
   */
  int getHealth() const { return _health; }

  /**
   * Gets the current attack cooldown of the enemy.
   *
   * @return the current health.
   */
  int getAttackCooldown() const { return _attack_cooldown; }

  /**
   * Sets the current enemy's health.
   *
   * @param value The current enemy health.
   */
  void setHealth(int value) { _health = value; }

  /**
   * Sets the attack cooldown.
   *
   * @param value The attack cooldown.
   */
  void setAttackCooldown(int value) { _attack_cooldown = value; }

  /**
   * Reduces the enemy's health.
   *
   * @param value The value to reduce the health by.
   */
  void reduceHealth(int value) { _health -= value; }

  /**
   * Reduces the enemy's attack cooldown.
   *
   * @param value The value to reduce the health by.
   */
  void reduceAttackCooldown(int value) { _attack_cooldown -= value; }

  /**
   * The enemy took damage.
   *
   */
  void takeDamage();

  /**
   * Returns the speed of the enemy.
   *
   * @return the enemy speed.
   */
  float getSpeed() const { return _speed; }

  /**
   * Add a bullet.
   *
   * @param p the position of the bullet to spawn in.
   */
  void addBullet(cugl::Vec2 p);

  /**
   * Deletes a bullet if needed.
   */
  void deleteProjectile(std::shared_ptr<cugl::physics2::ObstacleWorld> _world,
                        std::shared_ptr<cugl::scene2::SceneNode> _world_node);

  /**
   * Deletes all bullets.
   */
  void deleteAllProjectiles(
      std::shared_ptr<cugl::physics2::ObstacleWorld> _world,
      std::shared_ptr<cugl::scene2::SceneNode> _world_node);

  /**
   * Gets the enemy's projectiles.
   *
   * @return the projectiles the enemy has shot.
   */
  std::unordered_set<std::shared_ptr<Projectile>> getProjectiles() {
    return _projectiles;
  }

  /**
   * Set the current state of the enemy.  IDLE, ATTACKING, CHASING, AVOIDING...
   *
   * @param state The state the enemy should be set to.
   */
  void setCurrentState(State state) { _current_state = state; }

  /**
   * Get the current state of the enemy. IDLE, ATTACKING, CHASING, AVOIDING...
   *
   * @return The state of the enemy.
   */
  State getCurrentState() { return _current_state; }

  /**
   * Set the enemy type.
   *
   * @param state The enemy type in string.
   */
  void setType(std::string type);

  /**
   * Set the enemy type.
   *
   * @return The enemy type.
   */
  EnemyType getType() { return _enemy_type; }
  
  /**
   * Resets the sensors of the enemy.
   */
  void resetSensors();

#pragma mark -
#pragma mark Physics Methods
  /**
   * Creates the physics Body(s) for this object, adding them to the world.
   *
   * This method overrides the base method to keep your ship from spinning.
   *
   * @param world Box2D world to store body
   *
   * @return true if object allocation succeeded
   */
  void createFixtures() override;

  /**
   * Release the fixtures for this body, reseting the shape
   *
   * This is the primary method to override for custom physics objects.
   */
  void releaseFixtures() override;

  /**
   * Updates the object's physics state (NOT GAME LOGIC).
   *
   * We use this method to reset cooldowns.
   *
   * @param delta Number of seconds since last animation frame
   */
  void update(float dt) override;

  /**
   * Promise to change the physics state in the next update call.
   *
   * @param enable If the physics object should enabled or disabled.
   */
  void promiseToChangePhysics(bool enable) {
    _promise_to_change_physics = true;
    _promise_to_enable = enable;
  }

  /**
   * If this body has promised to change physics state.
   *
   * @return If the body has promised to change physics state.
   */
  bool getPromiseToChangePhysics() const { return _promise_to_change_physics; }

  /**
   * If this body has promised to enable or disable physics.
   *
   * @return If this body has promised to enable or disable physics.
   */
  bool getPromiseToEnable() const { return _promise_to_enable; }

#pragma mark Graphics

  /**
   * Sets the scene graph node representing this enemy.
   *
   * @param node  The scene graph node representing this enemy.
   */
  void setNode(const std::shared_ptr<cugl::Texture>& texture, std::shared_ptr<cugl::scene2::SceneNode> debug_node);

  /**
   * Gets the grunt scene graph node.
   *
   * @return node the node that has been set.
   */
  std::shared_ptr<cugl::scene2::SpriteNode>& getNode();

  /**
   * Sets the position of the room the enemy is in, for drawing purposes.
   *
   * @param pos The bottom left corner of the room the enemy is in.
   */
  void setRoomPos(cugl::Vec2 pos) { _room_pos = pos; }

#pragma mark Movement
  /**
   * Moves the enemy by the specified amount.
   *
   * @param forwardX Amount to move in the x direction.
   * @param forwardY Amount to move in the y direction.
   */
  void move(float forwardX, float forwardY);

  /**
   * Changes the direction of the enemy.
   *
   * @param facing_left is true if character should face left, false otherwise.
   */
  void setFacingLeft(bool facing_left);
};
#endif /* ENEMY_MODEL_H */
