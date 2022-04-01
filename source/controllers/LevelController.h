#ifndef CONTROLLERS_LEVEL_CONTROLLER_H_
#define CONTROLLERS_LEVEL_CONTROLLER_H_

#include <cugl/cugl.h>

#include "../generators/LevelGenerator.h"
#include "../models/LevelModel.h"
#include "Controller.h"

/**
 * A level controller that takes care of initializing the board model.
 */
class LevelController : public Controller {
 private:
  /** A reference to all game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** A reference to the world node. */
  std::shared_ptr<cugl::scene2::SceneNode> _world_node;
  /** A reference to the debug node. */
  std::shared_ptr<cugl::scene2::SceneNode> _debug_node;
  /** A reference to the box2d world. */
  std::shared_ptr<cugl::physics2::ObstacleWorld> _world;
  /** A level generator. */
  std::shared_ptr<level_gen::LevelGenerator> _level_gen;
  /** The level model. */
  std::shared_ptr<LevelModel> _level_model;
  /** The room to be removed after moving from neighboring tile. */
  std::shared_ptr<RoomModel> _room_on_chopping_block;
  /** The id of the next enemy to add, increasing each time. */
  int next_enemy_id = 0;

 public:
  /** Construct a new Level Controller */
  LevelController()
      : _assets(nullptr),
        _world_node(nullptr),
        _debug_node(nullptr),
        _world(nullptr),
        _level_gen(nullptr),
        _level_model(nullptr),
        _room_on_chopping_block(nullptr) {}
  /** Destroy this Level Controller */
  ~LevelController() { dispose(); }

  /**
   * Initialize the level controller.
   * @param assets The assets for the game.
   * @param world The scene2 node for the world.
   * @param level_gen The level generator.
   */
  bool init(const std::shared_ptr<cugl::AssetManager> &assets,
            const std::shared_ptr<cugl::scene2::SceneNode> &world_node,
            const std::shared_ptr<cugl::scene2::SceneNode> &debug_node,
            const std::shared_ptr<level_gen::LevelGenerator> &level_gen);

  /**
   * This method allocated a new level controller.
   * @param assets The assets for the game.
   * @param world The scene2 node for the world.
   * @param level_gen The level generator.
   */
  static std::shared_ptr<LevelController> alloc(
      const std::shared_ptr<cugl::AssetManager> &assets,
      const std::shared_ptr<cugl::scene2::SceneNode> &world_node,
      const std::shared_ptr<cugl::scene2::SceneNode> &debug_node,
      const std::shared_ptr<level_gen::LevelGenerator> &level_gen) {
    auto result = std::make_shared<LevelController>();
    if (result->init(assets, world_node, debug_node, level_gen)) return result;
    return nullptr;
  }

  /** Update the Level Controller. */
  void update(float timestep) override;

  /** Dispose of the level controller and all it's values. */
  void dispose() override;

  /** Change room given a door that was hit.
   * @param door_sensor_name The name of the door sensor that was hit.
   */
  void changeRoom(std::string &door_sensor_name);

  /**
   * Get the box2d world.
   * @return The box2d world for the game.
   */
  std::shared_ptr<cugl::physics2::ObstacleWorld> getWorld() const {
    return _world;
  }

  /**
   * Get the level model that contains all the rooms, player, enemies, etc...
   * @return The level model.
   */
  std::shared_ptr<LevelModel> getLevelModel() const { return _level_model; }

 private:
  /** Populate the level. */
  void populate();

  /**
   * Loop through all the rooms and find the bounds of the world.
   * Instantiates the world with the calculate bounds.
   */
  void instantiateWorld();

  /**
   * Instantiate all the door obstacles and apply the room references for every
   * room. Returns the unused doors.
   *
   * @param room The level generation room data.
   * @param room_model The room model for the game.
   * @return The unused doors.
   */
  std::vector<cugl::Vec2> instantiateDoors(
      const std::shared_ptr<level_gen::Room> &room,
      const std::shared_ptr<RoomModel> &room_model);

  /**
   * Cover the unused doors with a wall tile.
   *
   * @param room The level generation room data.
   * @param room_model The room model for the game.
   * @param unused_doors The list of unused door coordinates.
   */
  void coverUnusedDoors(const std::shared_ptr<level_gen::Room> &room,
                        const std::shared_ptr<RoomModel> &room_model,
                        std::vector<cugl::Vec2> &unused_doors);

  /**
   * Instantiate all the enemies for the room.
   *
   * @param room The level generation room data.
   * @param room_model The room model for the game.
   * @param enemies The list to put the enemies in.
   */
  void instantiateEnemies(const std::shared_ptr<level_gen::Room> &room,
                          const std::shared_ptr<RoomModel> &room_model,
                          std::vector<std::shared_ptr<EnemyModel>> &enemies);
};

#endif  // CONTROLLERS_LEVEL_CONTROLLER_H_
