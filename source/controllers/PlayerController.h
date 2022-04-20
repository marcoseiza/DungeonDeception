#ifndef CONTROLLERS_PLAYER_CONTROLLER_H_
#define CONTROLLERS_PLAYER_CONTROLLER_H_
#include <cugl/cugl.h>

#include "../models/Player.h"
#include "../models/Projectile.h"
#include "../models/Sword.h"
#include "Controller.h"
#include "InputController.h"

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
  bool init(const std::shared_ptr<Player>& player,
            const std::shared_ptr<cugl::AssetManager>& assets,
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
      const std::shared_ptr<Player>& player,
      const std::shared_ptr<cugl::AssetManager>& assets,
      const std::shared_ptr<cugl::physics2::ObstacleWorld>& world,
      const std::shared_ptr<cugl::scene2::SceneNode>& world_node,
      const std::shared_ptr<cugl::scene2::SceneNode>& debug_node) {
    std::shared_ptr<PlayerController> result =
        std::make_shared<PlayerController>();

    if (result->init(player, assets, world, world_node, debug_node)) {
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
                   const cugl::NetworkDeserializer::Message& msg);

  /**
   * Process the position of the player with the corresponding player_id in the
   * _players list.
   *
   * @param player_id The player ids
   * @param pos_x The updated player x position
   * @param pos_y The updated player y position
   */
  void processPlayerInfo(int player_id, int room_id, float pos_x, float pos_y);

  /** Update the projectiles. */
  void updateSlashes(float timestep);

  /** Linearly interpolate the player by the network positions. */
  void interpolate(float timestep, const std::shared_ptr<Player>& player);

  void move(float timestep);

  void attack();

  void addPlayer(const std::shared_ptr<Player>& player) {
    if (_players.find(player->getPlayerId()) == _players.end()) {
      _players[player->getPlayerId()] = player;
    }
  }

  std::shared_ptr<Player> getMyPlayer() { return _player; }

  std::shared_ptr<Player> getPlayer(int id) {
    if (_players.find(id) != _players.end()) {
      return _players[id];
    }
    return nullptr;
  }

  std::vector<std::shared_ptr<Player>> getPlayerList() {
    std::vector<std::shared_ptr<Player>> player_list;
    for (auto it : _players) player_list.push_back(it.second);
    return player_list;
  }

  std::unordered_map<int, std::shared_ptr<Player>> getPlayers() {
    return _players;
  }

  std::shared_ptr<Sword> getSword() { return _sword; }
};

#endif /* CONTROLLERS_PLAYER_CONTROLLER_H_ */
