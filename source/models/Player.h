#ifndef MODELS_PLAYER_H_
#define MODELS_PLAYER_H_

#include <cugl/cugl.h>
#include <stdio.h>

#include "Projectile.h"
#include "Sword.h"

class Player : public cugl::physics2::CapsuleObstacle {
 public:
  /** Enum for the player's state (for animation). */
  enum State { IDLE, MOVING, ATTACKING, DASHING };

 private:
  /** The scene graph node for the player (moving). */
  std::shared_ptr<cugl::scene2::SpriteNode> _player_node;

  /** The scene graph node for the player name (moving). */
  std::shared_ptr<cugl::scene2::Label> _name_node;

  /** The energy bar for the player name (moving). */
  std::shared_ptr<cugl::scene2::ProgressBar> _energy_bar;

  /** The corrupted energy bar for the player name (moving). */
  std::shared_ptr<cugl::scene2::ProgressBar> _corrupted_energy_bar;

  /** The block icon for the player (moving). */
  std::shared_ptr<cugl::scene2::SceneNode> _block_icon;
  /** A counter for flashing the block icon. */
  int _flash_block_icon_counter;

  /** Promise to move to this position in next update. */
  std::optional<cugl::Vec2> _promise_pos_cache;

  /** The last two positions the server has sent to this player. */
  std::array<cugl::Vec2, 2> _network_pos_cache;

  /** A timestamp that represents the last time the player position was set. */
  cugl::Timestamp _last_network_pos_udpate;

  /** The player's current state. */
  State _current_state;

  /** The player's role, true if betrayer, false otherwise */
  bool _is_betrayer;

  /** The player's unique id. */
  int _id;

  /** The player's display name. */
  std::string _display_name;

  /** The unique id of the room the player is in. */
  int _room_id;

  /** Player health. */
  int _health;

  /** Player luminance. */
  int _luminance;

  /** Amount of player luminance that has been corrupted. */
  int _corrupted_luminance;

  /** The list of players that have blocked this betrayer, mapped from the
   * player id to their countdown. */
  std::unordered_map<int, int> _blocked_players;

  /** Player energy. */
  float _energy;

  /** Amount of player energy that has been corrupted. */
  float _corrupted_energy;

  /** Force to be applied to the player. */
  cugl::Vec2 _force;

  /** Is this player dead? */
  bool _isDead;

  /** Is the player respawning? */
  bool _is_respawning;

  /** Represents the offset between the center of the player and the center of
   * the capsule obstacle. */
  cugl::Vec2 _offset_from_center;

  /** The list of slashes that have been released from the sword. */
  std::unordered_set<std::shared_ptr<Projectile>> _slashes;

  /** If the player is moving left (80), down (81), right (82), or up (83). */
  int _mv_direc;

  /** The move direction for the last frame. */
  cugl::Vec2 _last_move_dir;

  /** If basic display name and betrayer info has been sent to host. */
  bool _basic_info_sent_to_host;
  
  /** Represents the def for the damage area for the enemy. */
  b2FixtureDef _projectile_sensor_def;
  /** Represents the hit area for the enemy. */
  b2Fixture* _projectile_sensor;
  /** Keeps an instance of the name alive for collision detection. */
  std::shared_ptr<std::string> _projectile_sensor_name;

 public:
  /** Countdown to change animation frame. */
  int _frame_count;
  /** Countdown for attacking frames. */
  int _attack_frame_count;
  /** Countdown for hurting frames. */
  int _hurt_frames;
  /** Countdown for holding attack button. */
  int _hold_attack;
  /** Whether the player can make a sword slash. */
  bool _can_make_slash;
  /** Countdown for betrayer corrupt feedback. */
  int _corrupt_count;

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
  ~Player() { dispose(); }

  /**
   * Initializes a new player with the given position and name.
   *
   * @param  pos          Initial position in world coordinates.
   * @param  name         The name of the player (for Box2D).

   *
   * @return  true if the obstacle is initialized properly, false otherwise.
   */
  virtual bool init(const cugl::Vec2 pos, const std::string& name);

  /**
   * Dispose of this player and all its dependencies.
   */
  void dispose();

#pragma mark Static Constructors
  /**
   * Returns a new capsule object at the given point with no size.
   *
   * @param  pos          Initial position in world coordinates.
   * @param  name         The name of the player (for Box2D).
   *
   * @return a new capsule object at the given point with no size.
   */
  static std::shared_ptr<Player> alloc(const cugl::Vec2 pos,
                                       const std::string& name) {
    std::shared_ptr<Player> result = std::make_shared<Player>();
    return (result->init(pos, name) ? result : nullptr);
  }

#pragma mark Properties

  /**
   * Returns if the player has been hit.
   * @return if player has been hit.
   */
  bool isHit() const;

  /**
   * Returns the current state of the player.
   *
   * @return the current state.
   */
  State getState() const { return _current_state; }

  /**
   * Sets the current state of the player and changes textures accordingly.
   *
   * @param set current state.
   */
  void setState(State state) { _current_state = state; }

  /**
   * Returns if the player is a betrayer.
   *
   * @return true if player is betrayer, false otherwise.
   */
  bool isBetrayer() const { return _is_betrayer; }

  /**
   * Sets the role of the player as betrayer or not.
   *
   * @param set current role to betrayer if true, cooperator if false.
   */
  void setBetrayer(bool b) { _is_betrayer = b; }

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
   * Returns the current energy of the player.
   *
   * @return the current energy.
   */
  float getEnergy() const { return _energy; }

  /**
   * Sets the current player's energy.
   *
   * @param value The current player energy.
   */
  void setEnergy(float value) { _energy = std::min(value, 100.0f); }

  /**
   * Returns the current corrupted energy of the player.
   *
   * @return the current energy.
   */
  float getCorruptedEnergy() const { return _corrupted_energy; }

  /**
   * Turns regular energy into corrupted energy.
   *
   * @param value The amount of energy to be corrupted.
   */
  void turnEnergyCorrupted(float value) {
    _corrupted_energy += value;
    _corrupted_energy = std::min(_corrupted_energy, _energy);
  }

  /**
   * Manually set the corruption value.
   * @param value The value that is corrupted.
   */
  void setCorruptedEnergy(float value) {
    _corrupted_energy = std::min(value, 100.0f);
  }

  /** Sets the frames for player to turn orange to indicate corrupting. */
  void setCorrupted();

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
   * Returns if the player is dead;
   *
   * @return if player is dead.
   */
  bool getDead() const { return _isDead; }

  /**
   * Sets if the player is dead
   *
   * @param dead if player is dead.
   */
  void setDead(bool dead) { _isDead = dead; }

  /**
   * Returns if the player is respawnings;
   *
   * @return if player is respawning.
   */
  bool getRespawning() const { return _is_respawning; }

  /**
   * Sets if the player is respawning.
   *
   * @param respawning if player is respawning.
   */
  void setRespawning(bool respawning) { _is_respawning = respawning; }

  /**
   * Gets the player's movement direction.
   *
   * @return the movement direction
   */
  int getMoveDir() { return _mv_direc - 80; }

  /**
   * Returns the player's offset from the center.
   *
   * @return the offset from center.
   */
  cugl::Vec2 getOffset() const { return _offset_from_center; }

  /**
   * Gets the player's display name
   *
   * @return the display name
   */
  std::string getDisplayName() { return _display_name; }

  /**
   * Returns the player's slashes made.
   *
   * @return the slashes.
   */
  std::unordered_set<std::shared_ptr<Projectile>> getSlashes() {
    return _slashes;
  }

  /**
   * Returns the betrayer's players that have blocked this betrayer.
   *
   * @return the players that blocked this betrayer.
   */
  std::unordered_map<int, int> getBetrayersBlockedPlayers() {
    return _blocked_players;
  }

  /**
   * Add a player that blocked this betrayer.
   */
  void setBlockPlayerOnBetrayer(int runner_id);

  /**
   * Update the countdowns on the players that blocked this betrayer and remove
   * if needed.
   */
  void updateBlockedPlayers();

  /**
   * Internal method for getting the correct run high limit.
   *
   * @return the run high limit
   */
  int getRunHighLim();

  /**
   * Internal method for getting the correct run low limit.
   *
   * @return the run low limit
   */
  int getRunLowLim();

  /**
   * Check if the player is stepping on the floor using animation frames.
   *
   * @return If the player is stepping on the floor.
   */
  bool isSteppingOnFloor();

  /**
   * Internal method for getting the correct attack high limit.
   *
   * @return the attack high limit
   */
  int getAttackHighLim();
  
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
   * Update the scene graph.
   *
   * @param delta the timing value.
   */
  void update(float delta) override;

  /**
   * Set a position promise for the player. The player will move to this
   * position in the next update call. (Used for teleporting between rooms).
   *
   * @param pos The position the player promises to.
   */
  void setPosPromise(cugl::Vec2 pos) { _promise_pos_cache = pos; }

  /**
   * Get the player position promise. The player will move to this
   * position in the next update call. (Used for teleporting between rooms).
   *
   * @return The position promise currently set.
   */
  cugl::Vec2 getPosPromise() const { return *_promise_pos_cache; }

  /**
   * Set a network position promise for this player. This is used by the network
   * for interpolation of the current and true player positions.
   *
   * @param pos The network position.
   */
  void setNetworkPos(cugl::Vec2 pos) {
    _last_network_pos_udpate.mark();
    _network_pos_cache[1] = _network_pos_cache[0];
    _network_pos_cache[0] = pos;
  }

  /**
   * Get the last two positions given to this player by the network.
   *
   * @return The last two positions given to this player by the network.
   */
  std::array<cugl::Vec2, 2> getNetworkPosCache() { return _network_pos_cache; }

  /**
   * Get the time since the last packet for player position was received. In
   * milliseconds.
   *
   * @return The time since the last packet for player position was received. In
   * milliseconds.
   */
  float getTimeSinceLastNetworkPosUpdate() {
    cugl::Timestamp time;
    Uint32 millis =
        (Uint32)time.ellapsedMicros(_last_network_pos_udpate) / 1000;
    return (float)millis;
  }

  /**
   * Set that the player has sent all it's basic info to the host.
   */
  void setBasicInfoSentToHost(bool val) { _basic_info_sent_to_host = val; }

  /**
   * @return If basic info has been sent to host.
   */
  bool hasBasicInfoSentToHost() { return _basic_info_sent_to_host; }

#pragma mark Graphics

  /**
   * Sets the scene graph node representing this player.
   *
   * @param node      The scene graph node representing this player.
   */
  void setPlayerNode(const std::shared_ptr<cugl::scene2::SpriteNode>& node);

  /**
   * Set the display name for the character. Will display above the head.
   *
   * @param name The name to display
   */
  void setDisplayName(const std::string& name) { _display_name = name; }

  /**
   * Sets the scene graph node representing the name above the player.
   *
   * @param font The font for the name above the player.
   * @param display_betrayer  True if name should be shown in different color.
   */
  void setNameNode(const std::shared_ptr<cugl::Font>& font,
                   bool display_betrayer);

  /**
   * Sets the scene graph node representing the floating energy bar.
   * @param node The scene graph node representing the energy bar.
   */
  void setEnergyBar(const std::shared_ptr<cugl::scene2::ProgressBar>& bar);

  /**
   * Sets the scene graph node representing the floating corrupted energy bar.
   * @param node The scene graph node representing the corrupted energy bar.
   */
  void setCorruptedEnergyBar(
      const std::shared_ptr<cugl::scene2::ProgressBar>& bar);

  /**
   * Sets the scene graph node representing the floating block icon.
   * @param node The scene graph node representing the block icon.
   */
  void setBlockIcon(const std::shared_ptr<cugl::scene2::SceneNode>& icon);

  /**
   * Returns the scene graph node representing the floating block icon.
   * @return The scene graph node representing the block icon.
   */
  std::shared_ptr<cugl::scene2::SceneNode> getBlockIcon() {
    return _block_icon;
  }

  /** Flash the block icon red to make it clear to betrayers they can't infect
   * them. */
  void flashBlockIcon();

  /**
   * Gets the scene graph node representing this player's name.
   *
   * @return The scene graph node representing this player's name.
   */
  std::shared_ptr<cugl::scene2::Label> getNameNode() { return _name_node; }

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
   * Returns the player's room id.
   * @return the room id
   */
  int getRoomId() { return _room_id; }

  /**
   * Sets the player's room id.
   */
  void setRoomId(int id) { _room_id = id; }

  /**
   * Sets the frame of the animation.
   */
  void animate();

#pragma mark Movement

  /**
   * Moves the player by the specified amount.
   *
   * @param forward Amount to move in the x and y direction
   * @param speed The speed of movement.
   */
  void move(const cugl::Vec2& forward, float speed);

  /**
   * @return Returns the last move direction given to the player.
   */
  cugl::Vec2 getLastMoveDir() const { return _last_move_dir; }

  /**
   * Updates the directino the player sprite is facing based on changes in x and
   * y.
   *
   * @param diff The change in position
   */
  void updateDirection(const cugl::Vec2& diff);

  /**
   * Make a sword slash projectile.
   *
   * @param attackDir the direction to make the sword slash towards.
   * @param swordPos the position of the sword to make the slash originate.
   */
  void makeSlash(cugl::Vec2 attackDir, cugl::Vec2 swordPos);

  /**
   * Delete the sword slash.
   *
   * @param world  the world to delete the sword slash from.
   * @param world_node the world_node to delete the sword slash from.
   */
  void checkDeleteSlashes(std::shared_ptr<cugl::physics2::ObstacleWorld> world,
                          std::shared_ptr<cugl::scene2::SceneNode> world_node);
};
#endif /* PLAYER_H */
