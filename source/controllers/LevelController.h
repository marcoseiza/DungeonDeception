#ifndef CONTROLLERS_LEVEL_CONTROLLER_H_
#define CONTROLLERS_LEVEL_CONTROLLER_H_

#include <cugl/cugl.h>

#include "../generators/LevelGenerator.h"
#include "../models/EnemyModel.h"
#include "../models/LevelModel.h"
#include "Controller.h"
#include "ParticleController.h"
#include "PlayerController.h"

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
  /** A reference to map of this world. */
  std::shared_ptr<cugl::scene2::SceneNode> _map;
  /** A reference to the box2d world. */
  std::shared_ptr<cugl::physics2::ObstacleWorld> _world;
  /** A level generator. */
  std::shared_ptr<level_gen::LevelGenerator> _level_gen;
  /** A reference to the player controller for player access. */
  std::shared_ptr<PlayerController> _player_controller;
  /** The level model. */
  std::shared_ptr<LevelModel> _level_model;
  /** The room to be removed after moving from neighboring tile. */
  std::shared_ptr<RoomModel> _room_on_chopping_block;
  /** The id of the next enemy to add, increasing each time. */
  int _next_enemy_id;

  /** A reference to the particle controller. */
  std::shared_ptr<ParticleController> _particle_controller;

 public:
  /** Construct a new Level Controller */
  LevelController()
      : Controller(),
        _next_enemy_id(0),
        _assets(nullptr),
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
   * @param world_node The scene2 node for the world.
   * @param debug_node The scene2 node for the debug.
   * @param level_gen The level generator.
   * @param map The scene2 node for the map of this world.
   * @param is_betrayer If my player is a betrayer.
   */
  bool init(const std::shared_ptr<cugl::AssetManager> &assets,
            const std::shared_ptr<cugl::scene2::SceneNode> &world_node,
            const std::shared_ptr<cugl::scene2::SceneNode> &debug_node,
            const std::shared_ptr<level_gen::LevelGenerator> &level_gen,
            const std::shared_ptr<cugl::scene2::SceneNode> &map,
            bool is_betrayer);

  /**
   * This method allocated a new level controller.
   * @param assets The assets for the game.
   * @param world_node The scene2 node for the world.
   * @param debug_node The scene2 node for the debug.
   * @param level_gen The level generator.
   * @param map The scene2 node for the map of this world.
   * @param is_betrayer If my player is a betrayer.
   */
  static std::shared_ptr<LevelController> alloc(
      const std::shared_ptr<cugl::AssetManager> &assets,
      const std::shared_ptr<cugl::scene2::SceneNode> &world_node,
      const std::shared_ptr<cugl::scene2::SceneNode> &debug_node,
      const std::shared_ptr<level_gen::LevelGenerator> &level_gen,
      const std::shared_ptr<cugl::scene2::SceneNode> &map, bool is_betrayer) {
    auto result = std::make_shared<LevelController>();
    if (result->init(assets, world_node, debug_node, level_gen, map,
                     is_betrayer))
      return result;
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

  /** Change room and move to the center.
   * @param destination_room_id The room destination id
   */
  void moveToCenterOfRoom(int destination_room_id);

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

  /**
   * Set the player controller for player access.
   *
   * @param player_controller The player controller.
   */
  void setPlayerController(
      const std::shared_ptr<PlayerController> &player_controller) {
    _player_controller = player_controller;
  }

  /**
   * Get an enemy by id.
   *
   * @param enemy_id The enemy id.
   * @return The enemy found, or nullptr if it does not exist.
   */
  std::shared_ptr<EnemyModel> getEnemy(int enemy_id);

  /**
   * Set the particle controller for particles.
   * @param controller The particle controller.
   */
  void setParticleController(
      const std::shared_ptr<ParticleController> &controller) {
    _particle_controller = controller;
  }

 private:
  /**
   * Change the current room in the map node.
   * @param room_id The room id of the room to change to.
   */
  void updateMapCurrentRoom(int room_id);

  /** Populate the level. */
  void populate();

  /** Set up map for gameplay. */
  void setupMap(bool is_betrayer);

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
