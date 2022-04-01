#ifndef CONTROLLERS_PLAYER_CONTROLLER_H_
#define CONTROLLERS_PLAYER_CONTROLLER_H_
#include <cugl/cugl.h>

#include "../models/Player.h"
#include "../models/Projectile.h"
#include "../models/Sword.h"
#include "Controller.h"

/**
 * A class to handle enemy AI.
 */
class PlayerController {
 protected:
  /** Reference to the player this controls. */
  std::shared_ptr<Player> _player;
  /** The slash texture. */
  std::shared_ptr<cugl::Texture> _slash_texture;
  /** A reference to the world node. */
  std::shared_ptr<cugl::scene2::SceneNode> _world_node;
  /** A reference to the debug node. */
  std::shared_ptr<cugl::scene2::SceneNode> _debug_node;
  /** A reference to the box2d world for adding projectiles */
  std::shared_ptr<cugl::physics2::ObstacleWorld> _world;

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
  bool init(std::shared_ptr<Player> player,
            std::shared_ptr<cugl::AssetManager> assets,
            std::shared_ptr<cugl::physics2::ObstacleWorld> world,
            std::shared_ptr<cugl::scene2::SceneNode> world_node,
            std::shared_ptr<cugl::scene2::SceneNode> debug_node);

  /**
   * Disposes the controller.
   */
  void dispose() {
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
      std::shared_ptr<Player> player,
      std::shared_ptr<cugl::AssetManager> assets,
      std::shared_ptr<cugl::physics2::ObstacleWorld> world,
      std::shared_ptr<cugl::scene2::SceneNode> world_node,
      std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
    std::shared_ptr<PlayerController> result =
        std::make_shared<PlayerController>();

    if (result->init(player, assets, world, world_node, debug_node)) {
      return result;
    }
    return nullptr;
  }

#pragma mark Properties

  /** Update the enemy. */
  void update(float timestep, cugl::Vec2 forward, bool didAttack, bool didDash, bool holdAttack,
              std::shared_ptr<Sword> sword);

  /** Update the projectiles. */
  void updateSlashes(float timestep);

  void move(float timestep, bool didDash, cugl::Vec2 forward);

  void attack(bool didAttack, bool holdAttack, std::shared_ptr<Sword> sword);
};

#endif /* CONTROLLERS_PLAYER_CONTROLLER_H_ */
