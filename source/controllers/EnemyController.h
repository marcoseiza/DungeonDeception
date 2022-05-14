#ifndef CONTROLLERS_ENEMY_CONTROLLER_H_
#define CONTROLLERS_ENEMY_CONTROLLER_H_
#include <cugl/cugl.h>
#include <stdlib.h>
#include <time.h>

#include "../models/EnemyModel.h"
#include "../models/Player.h"
#include "RayCastController.h"
#include "SoundController.h"

/**
 * A class to handle enemy AI.
 */
class EnemyController {
 protected:
  /** The projectile texture. */
  std::shared_ptr<cugl::Texture> _projectile_texture;
  /** A reference to the world node. */
  std::shared_ptr<cugl::scene2::SceneNode> _world_node;
  /** A reference to the debug node. */
  std::shared_ptr<cugl::scene2::SceneNode> _debug_node;
  /** A reference to the box2d world for adding projectiles */
  std::shared_ptr<cugl::physics2::ObstacleWorld> _world;
  /** A reference to the sound controller. */
  std::shared_ptr<SoundController> _sound_controller;

  /** Cache set for vertices to update the polys. */
  std::vector<cugl::Vec2> _vertices_cache;
  /** Cache array for weights. */
  std::array<float, 16> _weights;
  /** Cache vector for objects hit by ray cast. */
  std::unordered_map<cugl::physics2::Obstacle*, cugl::Vec2> _objects;
  /** Cache array for CW/CCW directions. */
  std::array<bool, 16> _cw_direcs;
  /** Direction to move the enemy model according to the weights. */
  cugl::Vec2 _direction;
  /** Timer for doing ray casting. */
  int _timer;

  /** A generator for random numbers. */
  std::default_random_engine _generator;

 public:
#pragma mark Constructors
  /** Creates a new enemy controller with the default settings. */
  EnemyController();

  /** Disposses this enemy controller, releasing all resources. */
  ~EnemyController() {}

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
  bool init(std::shared_ptr<cugl::AssetManager> assets,
            std::shared_ptr<cugl::physics2::ObstacleWorld> world,
            std::shared_ptr<cugl::scene2::SceneNode> world_node,
            std::shared_ptr<cugl::scene2::SceneNode> debug_node);

  /**
   * Disposes the controller.
   */
  void dispose() { _projectile_texture = nullptr; }

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
  static std::shared_ptr<EnemyController> alloc(
      std::shared_ptr<cugl::AssetManager> assets,
      std::shared_ptr<cugl::physics2::ObstacleWorld> world,
      std::shared_ptr<cugl::scene2::SceneNode> world_node,
      std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
    std::shared_ptr<EnemyController> result =
        std::make_shared<EnemyController>();

    if (result->init(assets, world, world_node, debug_node)) {
      return result;
    }
    return nullptr;
  }

#pragma mark Properties

  /** Update the enemy. */
  void update(bool is_host, float timestep, std::shared_ptr<EnemyModel> enemy,
              std::vector<std::shared_ptr<Player>> players, int room_id);
  
  /** Update the enemy (client side). */
  void updateIfClient(float timestep, std::shared_ptr<EnemyModel> enemy, int room_id);

  /** Change the enemy state. */
  virtual void changeStateIfApplicable(std::shared_ptr<EnemyModel> enemy,
                                       float distance) {}

  /** Perform the action according to the enemy state. */
  virtual void performAction(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p) {}

  /** Update the projectiles. */
  void updateProjectiles(float timestep, std::shared_ptr<EnemyModel> enemy);

  /** Figure out the weights for the enemy. */
  void findWeights(std::shared_ptr<EnemyModel> enemy,
                   std::shared_ptr<Player> player);

  /** Animate the enemy. */
  virtual void animate(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p) {}

#pragma mark Movement
 protected:
  /** Chase the player.
   *
   * @param p the player position.
   */
  virtual void chasePlayer(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p);

  /** Attack the player.
   *
   * @param p the player position.
   */
  virtual void attackPlayer(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p);

  /** Avoid the player.
   *
   * @param p the player position.
   */
  virtual void avoidPlayer(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p);

  /** Idle.
   */
  virtual void idling(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p);

  /** Stunned.
   */
  virtual void stunned(std::shared_ptr<EnemyModel> enemy);
};

#endif /* CONTROLLERS_ENEMY_CONTROLLER_H_ */
