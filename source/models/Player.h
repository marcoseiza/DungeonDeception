#ifndef MODELS_PLAYER_H_
#define MODELS_PLAYER_H_

#include <cugl/cugl.h>
#include <stdio.h>

#include "Sword.h"

class Player : public cugl::physics2::CapsuleObstacle {
  public:
  /** Enum for the player's state (for animation). */
  enum State { IDLE, MOVING, ATTACKING };
  
 private:

  /** The scene graph node for the player (moving). */
  std::shared_ptr<cugl::scene2::SpriteNode> _player_node;

  /** Promise to move to this position in next update. */
  cugl::Vec2 _promise_pos_cache;
  /** A boolean representing if we should move to the promise pos cache next
   * udpate call. */
  bool _promise;

  /** The player's current state. */
  State _current_state;

  /** The player's unique id. */
  int _id;

  /** Player health. */
  int _health;

  /** Force to be applied to the player. */
  cugl::Vec2 _force;
  /** Countdown to change animation frame. */
  int _frame_count;
  /** Countdown for attacking frames. */
  int _attack_frame_count;
  /** Countdown for hurting frames. */
  int _hurt_frames;

  /** Is this player dead? */
  bool isDead;

  /** Represents the offset between the center of the player and the center of
   * the capsule obstacle. */
  cugl::Vec2 _offset_from_center;

 public:
  
#pragma mark Constructors
  /**
   * Creates a player with the given position and data.
   *
   * @param pos The player position
   * @param data The data defining the player
   */
  Player(void) : CapsuleObstacle() {}

  /**
   * Disposes the player.
   */
  ~Player() {}

  /**
   * Initializes a new player with the given position and name.
   *
   * @param  pos      Initial position in world coordinates.
   * @param  name       The name of the player.
   *
   * @return  true if the obstacle is initialized properly, false otherwise.
   */
  virtual bool init(const cugl::Vec2 pos, string name);

#pragma mark Static Constructors
  /**
   * Returns a new capsule object at the given point with no size.
   *
   * @param pos   Initial position in world coordinates.
   *
   * @return a new capsule object at the given point with no size.
   */
  static std::shared_ptr<Player> alloc(const cugl::Vec2 pos, string name) {
    std::shared_ptr<Player> result = std::make_shared<Player>();
    return (result->init(pos, name) ? result : nullptr);
  }

#pragma mark Properties

  /**
   * Returns the current state of the player.
   *
   * @return the current state.
   */
  State getState() const { return _current_state; }

  /**
   * Sets the current state of the player and changes textures accordingly.
   *
   * @param state current state.
   */
  void setState(State state) { _current_state = state; }

  /**
   * Returns the current health of the player.
   *
   * @return the current health.
   */
  int getHealth() const { return _health; }

  /**
   * Sets the current player's health.
   *
   * @param value The current player health.
   */
  void setHealth(int value) { _health = value; }

  /**
   * Reduce health by value.
   *
   * @param value The value to reduce health by.
   */
  void reduceHealth(int value) { _health -= value; }

  /**
   * Actions for when the player is hit by the grunt.
   */
  void takeDamage();

  /**
   * When a player dies
   */
  void dies();

  /**
   * Update the scene graph.
   *
   * @param delta the timing value.
   */
  void update(float delta);

  /**
   * Set a position promise for the player. The player will move to this
   * position in the next update call. (Used for teleporting between rooms).
   *
   * @param pos The position the player promises to.
   */
  void setPosPromise(cugl::Vec2 pos) {
    _promise_pos_cache = pos;
    _promise = true;
  }

  /**
   * Get the player position promise. The player will move to this
   * position in the next update call. (Used for teleporting between rooms).
   *
   * @return The position promise currently set.
   */
  cugl::Vec2 getPosPromise() const { return _promise_pos_cache; }

#pragma mark Graphics

  /**
   * Sets the scene graph node representing this player.
   *
   * @param node  The scene graph node representing this player.
   */
  void setPlayerNode(const std::shared_ptr<cugl::scene2::SpriteNode> &node) {
    _player_node = node;
  }

  /**
   * Gets the player scene graph node.
   */
  std::shared_ptr<cugl::scene2::SpriteNode> getPlayerNode() {
    return _player_node;
  }

  /**
   * Returns the player id.
   * @return the player id
   */
  int getPlayerId() { return _id; }

  /**
   * Sets the player id.
   */
  void setPlayerId(int player_id) { _id = player_id; }

  /**
   * Sets the frame of the animation.
   *
   * @param forwardX Amount to move in the x direction
   * @param forwardY Amount to move in the y direction
   */
  void animate(cugl::Vec2 forward) { animate(forward.x, forward.y); }

  /**
   * Sets the frame of the animation.
   *
   * @param forwardX Amount to move in the x direction
   * @param forwardY Amount to move in the y direction
   */
  void animate(float forwardX, float forwardY);

#pragma mark Movement
  /**
   * Updates the player.
   */
  void step(float timestep, cugl::Vec2 forward, bool didAttack,
            std::shared_ptr<Sword> sword);

  /**
   * Moves the player by the specified amount.
   *
   * @param forward Amount to move in the x and y direction
   */
  void move(cugl::Vec2 forward) { move(forward.x, forward.y); }

  /**
   * Moves the player by the specified amount.
   *
   * @param forwardX Amount to move in the x direction.
   * @param forwardY Amount to move in the y direction.
   */
  void move(float forwardX, float forwardY);

  /**
   * Attacks if the player hit the attack button.
   *
   * @param didAttack If the player attacked.
   */
  void attack(bool didAttack, std::shared_ptr<Sword> sword);
};
#endif /* PLAYER_H */
