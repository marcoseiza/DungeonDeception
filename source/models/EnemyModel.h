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
    SKIRTING,
    /** The enemy is knocked back. */
    STUNNED,
    /** The enemy wants to move away from the player for a little. */
    MOVING_BACK,
    /** The enemy is wandering. */
    WANDER,
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
  /** Initial position of the enemy. */
  cugl::Vec2 _init_pos;
  
  /** The current state of the enemy. */
  State _current_state;

  /** The enemy type of this enemy. */
  EnemyType _enemy_type;

  /** The size of the enemy. */
  cugl::Vec2 _size;

  /** Enemy unique id. */
  int _id;

  /** Room id. */
  int _room_id;

  /** Enemy health. */
  int _health;

  /** Enemy speed. */
  float _speed;

  /** The scene graph node for the enemy. */
  std::shared_ptr<cugl::scene2::SceneNode> _enemy_node;

  /** Represents the def for the hit area for the enemy. */
  b2FixtureDef _hitbox_sensor_def;
  /** Represents the hit area for the enemy. */
  b2Fixture* _hitbox_sensor;
  /** Keeps an instance of the name alive for collision detection. */
  std::shared_ptr<std::string> _hitbox_sensor_name;
  /** The node for debugging the hitbox sensor */
  std::shared_ptr<cugl::scene2::WireNode> _hitbox_sensor_node;

  /** Represents the def for the damage area for the enemy. */
  b2FixtureDef _damage_sensor_def;
  /** Represents the hit area for the enemy. */
  b2Fixture* _damage_sensor;
  /** Keeps an instance of the name alive for collision detection. */
  std::shared_ptr<std::string> _damage_sensor_name;
  /** The node for debugging the damage sensor */
  std::shared_ptr<cugl::scene2::WireNode> _damage_sensor_node;
  
  /** Another fixture def to ensure enemy doesn't go through walls when attacking. */
  b2FixtureDef _wall_fixture_def;
  /** Fixture for wall. */
  b2Fixture* _wall_fixture;
  /** The node for debugging the wall fixture */
  std::shared_ptr<cugl::scene2::WireNode> _wall_fixture_node;

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
  
  /** If the enemy attacked via dashing this update step. */
  bool _did_attack;
  
  /** Player position to attack in. */
  cugl::Vec2 _attack_dir;
  
  /** Attack position at the beginning of attack. */
  cugl::Vec2 _attack_init_pos;

 public:
  /** The set of polygon nodes corresponding to the weights for the direction of
   * the enemy. */
  std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> _polys;

  /** Whether the enemy is moving in a CW motion (when attacking). */
  bool _move_CW;

  /** Timer for switching from attack to chase. */
  int _atc_timer;

  /** Timer for switching from chase to attack. */
  int _cta_timer;

  /** Whether the enemy was knock backed. */
  bool _isKnockbacked;

  /** Timer for being stunned. */
  int _stunned_timer;

  /** Knockback direction. */
  cugl::Vec2 _knockback_dir;
  
  /** The count for switching to the next frame. */
  int _frame_count;

  /** The goal frame for the turtle enemy*/
  int _goal_frame;
  
  /** Timer for staying in the move back state. */
  int _move_back_timer;
  
  /** Timer for attempting to return/wander to original position. */
  int _wander_timer;

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
        _damage_sensor_name(nullptr),
        _wall_fixture(nullptr) {}

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
    _wall_fixture = nullptr;
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
   * Returns if enemy has been hit.
   * @return If the enemy has been hit.
   */
  bool isHit() const;
  
  /**
   * Returns the initial position of the enemy.
   */
  cugl::Vec2 getInitPos() const { return _init_pos; }

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
   * Returns the room id of the enemy.
   * @return the enemy room id.
   */
  int getRoomId() const { return _id; }

  /**
   * Sets the enemy's room id
   * @param id The room id.
   */
  void setRoomId(int id) { _room_id = id; }

  /**
   * Returns whether the enemy is facing left.
   *
   * @return direction enemy facing.
   */
  bool getFacingLeft() { return _facing_left; }

  /**
   * Sets whether the enemy is facing left
   *
   * @param id Whether the enemy is facing left.
   */
  void setFacingLeft(bool left) { _facing_left = left; }

  /**
   * Returns the current health of the enemy.
   *
   * @return the current health.
   */
  int getHealth() const { return _health; }

  /**
   * Sets the current enemy's health.
   *
   * @param value The current enemy health.
   */
  void setHealth(int value) { _health = value; }
  
  /**
   * Gets the current attack cooldown of the enemy.
   *
   * @return the current health.
   */
  int getAttackCooldown() const { return _attack_cooldown; }

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
  void takeDamage(float amount = 20);
  
  /**
   * The enemy took damage.
   *
   */
  void takeDamageWithKnockback(const cugl::Vec2 p, float amount = 20);

  /**
   * Returns the speed of the enemy.
   *
   * @return the enemy speed.
   */
  float getSpeed() const { return _speed; }
  
  /**
   * Set whether the enemy attacked.
   */
  void setAttack(bool val) { _did_attack = val; }
  
  /**
   * Returns whether the enemy attacked.
   *
   * @return whether the enemy attacked.
   */
  bool didAttack() const { return _did_attack; }

  /**
   * Resets info about whether an attack happened.
   */
  void clearAttackState() {
    _did_attack = false;
  }
  
  /**
   * Set the direction of the attack.
   */
  void setAttackDir(const cugl::Vec2 v) { _attack_dir = v; }
  
  /**
   * Returns the direction of the attack.
   */
  cugl::Vec2 getAttackDir() const { return _attack_dir; }
  
  /** Saves the enemy position at the start of an attack. */
  void setAttackInitPos(const cugl::Vec2 v) { _attack_init_pos = v; }
  
  /** Returns the direction of the attack. */
  cugl::Vec2 getAttackInitPos() const { return _attack_init_pos; }
  
  /**
   * Add a bullet.
   *
   * @param p the position of the bullet to spawn in.
   */
  void addBullet(cugl::Vec2 p);
  
  /**
   * Perform attack action according to enemy type.
   *
   * Should only be called by clients.
   *
   * @param dir The direction of the bullet, if making a bullet.
   */
  void performAttackAction(const cugl::Vec2 dir);

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
  
  /** When grunt or tank attacking, set the filter to only perform collisions on walls. */
  void setAttackingFilter();

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

#pragma mark Graphics

  /**
   * Sets the scene graph node representing this enemy.
   *
   * @param node  The scene graph node representing this enemy.
   */
  void setNode(const std::shared_ptr<cugl::Texture>& texture,
               std::shared_ptr<cugl::scene2::SceneNode> debug_node);

  /**
   * Gets the enemy scene graph node.
   *
   * @return node the node that has been set.
   */
  std::shared_ptr<cugl::scene2::SceneNode>& getNode();

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
   * Knocks back the enemy.
   */
  void knockback(const cugl::Vec2 p);

  /**
   * Returns whether or not the enemy is knocked back.
   */
  bool isKnockbacked() { return _isKnockbacked; };

  /**
   * Sets the knockbacked state of the enemy.
   */
  void setKnockbacked(bool isKnockbacked) { _isKnockbacked = isKnockbacked; };
};
#endif /* ENEMY_MODEL_H */
