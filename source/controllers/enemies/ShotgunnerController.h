#ifndef CONTROLLERS_SHOTGUNNER_CONTROLLER_H_
#define CONTROLLERS_SHOTGUNNER_CONTROLLER_H_
#include <cugl/cugl.h>

#include "../../models/EnemyModel.h"
#include "../../models/Player.h"
#include "../EnemyController.h"

/**
 * A class to handle enemy AI.
 */
class ShotgunnerController : public EnemyController {
 public:
#pragma mark Constructors
  /**
   * Creates a new AI controller with the default settings.
   */
  ShotgunnerController();

  /**
   * Disposses this input controller, releasing all resources.
   */
  ~ShotgunnerController() {}

  /**
   * Initializes a new shotgunner Controller.
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
  static std::shared_ptr<ShotgunnerController> alloc(
      std::shared_ptr<cugl::AssetManager> assets,
      std::shared_ptr<cugl::physics2::ObstacleWorld> world,
      std::shared_ptr<cugl::scene2::SceneNode> world_node,
      std::shared_ptr<cugl::scene2::SceneNode> debug_node) {
    std::shared_ptr<ShotgunnerController> result =
        std::make_shared<ShotgunnerController>();

    if (result->init(assets, world, world_node, debug_node)) {
      return result;
    }
    return nullptr;
  }

#pragma mark Properties

  /** Change the enemy state. */
  void changeStateIfApplicable(std::shared_ptr<EnemyModel> enemy,
                               float distance) override;

  /** Perform the action according to the enemy state. */
  void performAction(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p) override;

  /**
   * Animate the shotgunner.
   */
  void animate(std::shared_ptr<EnemyModel> enemy) override;

  /** Attack the player.
   *
   * @param p the player position.
   */
  void attackPlayer(std::shared_ptr<EnemyModel> enemy, cugl::Vec2 p) override;
  
  /** Update and attack the player if a client.
   *
   * @param enemy the enemy.
   */
  void clientUpdateAttackPlayer(std::shared_ptr<EnemyModel> enemy) override;
};

#endif /* CONTROLLERS_SHOTGUNNER_CONTROLLER_H_ */
