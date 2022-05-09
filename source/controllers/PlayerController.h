#ifndef CONTROLLERS_PLAYER_CONTROLLER_H_
#define CONTROLLERS_PLAYER_CONTROLLER_H_
#include <cugl/cugl.h>

#include "../models/Player.h"
#include "../models/Projectile.h"
#include "../models/Sword.h"
#include "../network/CustomNetworkSerializer.h"
#include "Controller.h"
#include "InputController.h"
#include "SoundController.h"
#include "TrailManager.h"

/**
 * A class to handle enemy AI.
 */
class PlayerController : public Controller {
 protected:
  /** The sword. */
  std::shared_ptr<Sword> _sword;
  /** Reference to the player this controls. */
  std::shared_ptr<Player> _player;

  /** A list of all the players in the game. */
  std::unordered_map<int, std::shared_ptr<Player>> _players;
  std::unordered_map<int, std::shared_ptr<TrailManager>> _trail_managers;

  /** The slash texture. */
  std::shared_ptr<cugl::Texture> _slash_texture;
  /** A reference to the world node. */
  std::shared_ptr<cugl::scene2::SceneNode> _world_node;
  /** A reference to the debug node. */
  std::shared_ptr<cugl::scene2::SceneNode> _debug_node;
  /** A reference to the box2d world for adding projectiles */
  std::shared_ptr<cugl::physics2::ObstacleWorld> _world;
  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** A reference to the sound controller. */
  std::shared_ptr<SoundController> _sound_controller;

 public:
#pragma mark Constructors
  /** Creates a new enemy controller with the default settings. */
  PlayerController();

  /** Disposses this enemy controller, releasing all resources. */
  ~PlayerController() {}

  /**
   * Initializes a new Enemy Controller.
   *
   * @param assets The asset manager for the game.
   * @param world The asset manager for the game.
   * @param world_node The world node for drawing the game.
   * @param debug_node The debug node for drawing the debug tools.
   *
   * @return true if the obstacle is initialized properly, false otherwise.
   */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets,
            const std::shared_ptr<cugl::physics2::ObstacleWorld>& world,
            const std::shared_ptr<cugl::scene2::SceneNode>& world_node,
            const std::shared_ptr<cugl::scene2::SceneNode>& debug_node);

  /**
   * Disposes the controller.
   */
  void dispose() override {
    _player = nullptr;
    _world_node = nullptr;
    _debug_node = nullptr;
    _world = nullptr;
    _trail_managers.clear();
  }

  /**
   * Set the sound controller for sound effects.
   * @param controller The sound controller.
   */
  void setSoundController(const std::shared_ptr<SoundController>& controller) {
    _sound_controller = controller;
  }

#pragma mark Static Constructors
  /**
   * Returns a new enemy controller.
   *
   * @param assets The asset manager for the game.
   * @param world The asset manager for the game.
   * @param world_node The world node for drawing the game.
   * @param debug_node The debug node for drawing the debug tools.
   *
   * @return a new capsule object at the given point with no size.
   */
  static std::shared_ptr<PlayerController> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets,
      const std::shared_ptr<cugl::physics2::ObstacleWorld>& world,
      const std::shared_ptr<cugl::scene2::SceneNode>& world_node,
      const std::shared_ptr<cugl::scene2::SceneNode>& debug_node) {
    std::shared_ptr<PlayerController> result =
        std::make_shared<PlayerController>();

    if (result->init(assets, world, world_node, debug_node)) {
      return result;
    }
    return nullptr;
  }

#pragma mark Properties

  /** Update the enemy. */
  void update(float timestep) override;

  /**
   * Processes data sent over the network.
   *
   * @param code The message code
   * @param msg The deserialized message
   */
  void processData(const Sint32& code,
                   const cugl::CustomNetworkDeserializer::CustomMessage& msg);

  /**
   * Process the position of the player with the corresponding player_id in the
   * _players list.
   *
   * @param player_id     The player ids
   * @param room_id       The room the player is currently in.
   * @param pos           The updated player position
   *
   */
  void processPlayerInfo(int player_id, int room_id, cugl::Vec2& pos);

  /**
   * Process the unimportant parts of the player's data with the corresponding
   * player_id in the _players list.
   *
   * @param player_id     The player ids
   * @param energy        The amount of energy the player has.
   * @param corruption    The amount of corruption the player has
   *
   */
  void processPlayerOtherInfo(int player_id, int energy, int corruption);

  /**
   * Process the basic player info that is only sent once, like display name and
   * betrayer state.
   *
   * @param player_id     The player ids
   * @param display_name  The name of this player
   * @param is_betrayer   True if player is a betrayer, false otherwise
   * @param energy        The amount of energy
   * @param corrupted     The amount of corrupted energy
   */
  void processBasicPlayerInfo(int player_id, const std::string& display_name,
                              bool is_betrayer);

  /** Update the projectiles. */
  void updateSlashes(float timestep);

  /** Linearly interpolate the player by the network positions. */
  void interpolate(float timestep, const std::shared_ptr<Player>& player);

  void move(float timestep);

  void attack();

  /** If player is a betrayer and they get blocked, blocks their corrupt button.
   */
  void blockCorrupt();

  void addPlayer(const std::shared_ptr<Player>& player) {
    if (_players.find(player->getPlayerId()) == _players.end()) {
      _players[player->getPlayerId()] = player;
    }
  }

  /**
   * Remove the given player and all its dependencies from the controller
   *
   * @param id The id of the player to remove.
   */
  void removePlayer(int id);

  std::shared_ptr<Player> makePlayer(int player_id);

  std::shared_ptr<Player> getMyPlayer() { return _player; }

  void setMyPlayer(const std::shared_ptr<Player>& player) { _player = player; }

  std::shared_ptr<Player> getPlayer(int id) {
    if (_players.find(id) != _players.end()) {
      return _players[id];
    }
    return nullptr;
  }

  std::shared_ptr<Player> getPlayerOrMakePlayer(int id) {
    if (_players.find(id) != _players.end()) {
      return _players[id];
    }
    return makePlayer(id);
  }

  std::vector<std::shared_ptr<Player>> getPlayerList() {
    std::vector<std::shared_ptr<Player>> player_list;
    for (auto it : _players) player_list.push_back(it.second);
    return player_list;
  }

  std::unordered_map<int, std::shared_ptr<Player>> getPlayers() {
    return _players;
  }

  /**
   * Get the number of betrayers currently in the game.
   * @return The number of betrayers.
   */
  int getNumberBetrayers() {
    int num_betrayers = 0;
    for (auto it : _players) {
      if (it.second->isBetrayer()) num_betrayers++;
    }
    return num_betrayers;
  }

  std::shared_ptr<Sword> getSword() { return _sword; }

 private:
  /**
   * Internal method to add a trail manager for the given player.
   *
   * @param player The given player to add a trail manager for.
   */
  void addTrailManager(const std::shared_ptr<Player>& player);
};

#endif /* CONTROLLERS_PLAYER_CONTROLLER_H_ */
