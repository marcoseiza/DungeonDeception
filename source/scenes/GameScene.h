#ifndef SCENES_GAME_SCENE_H_
#define SCENES_GAME_SCENE_H_
#include <box2d/b2_world_callbacks.h>
#include <cugl/cugl.h>

#include "../controllers/Controller.h"
#include "../controllers/InputController.h"
#include "../controllers/LevelController.h"
#include "../controllers/enemies/GruntController.h"
#include "../controllers/enemies/ShotgunnerController.h"
#include "../controllers/enemies/TankController.h"
#include "../controllers/enemies/TurtleController.h"
#include "../generators/LevelGenerator.h"
#include "../models/Player.h"

class GameScene : public cugl::Scene2 {
  /** The asset manager for loading. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** The network connection (as made by this scene). */
  std::shared_ptr<cugl::NetworkConnection> _network;

  /** The player.  */
  std::shared_ptr<Player> _my_player;

  /** The list of all players. */
  std::vector<std::shared_ptr<Player>> _players;

  /** The sword. */
  std::shared_ptr<Sword> _sword;

  /** Reference to the physics root of the scene graph. */
  std::shared_ptr<cugl::scene2::SceneNode> _world_node;

  /** Reference to the debug root of the scene graph */
  std::shared_ptr<cugl::scene2::SceneNode> _debug_node;

  /** The Box2d world */
  std::shared_ptr<cugl::physics2::ObstacleWorld> _world;

  /** The grunt controller for the game. */
  std::shared_ptr<GruntController> _grunt_controller;
  /** The shotgunner controller for the game. */
  std::shared_ptr<ShotgunnerController> _shotgunner_controller;
  /** The tank controller for the game. */
  std::shared_ptr<TankController> _tank_controller;
  /** The turtle controller for the game. */
  std::shared_ptr<TurtleController> _turtle_controller;

  /** The level controller for the game*/
  std::shared_ptr<LevelController> _level_controller;

  /** The controllers for the game */
  std::vector<std::shared_ptr<Controller>> _controllers;
  
  /** The serializer used to serialize complex data to send through the network.
   */
  cugl::NetworkSerializer _serializer;

  /** The deserializer used to deserialize complex data sent through the
   * network. */
  cugl::NetworkDeserializer _deserializer;

  /** Whether this player is the host. */
  bool _ishost;

  /** Whether we quit the game. */
  bool _quit;

  /** The number of terminals in the world. */
  int _num_terminals;

  /** The number of terminals activated in the world. */
  int _num_terminals_activated;

 public:
  GameScene() : cugl::Scene2() {}

  /**
   * Disposes of all resources allocated to this mode.
   */
  ~GameScene() { dispose(); }

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  void dispose() override;

  /**
   * Initializes the controller contents, and starts the game.
   *
   * @param assets    The (loaded) assets for this game mode.
   * @param level_gen The generated level.
   *
   * @return true if the controller is initialized properly, false otherwise.
   */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets,
            const std::shared_ptr<level_gen::LevelGenerator>& level_gen);

  /**
   * Sets whether debug mode is active.
   *
   * If true, all objects will display their physics bodies.
   *
   * @param value whether debug mode is active.
   */
  void setDebug(bool value) { _debug_node->setVisible(value); }

  /**
   * Populate the scene with the Box2D objects.
   *
   * @param dim The dimensions of the screen.
   */
  void populate(cugl::Size dim);

  /**
   * Check if the cooperators have won.
   *
   * @return true if at least half of the terminals have been activated.
   */
  bool checkCooperatorWin();

  /**
   * The method called to update the game mode.
   * This method contains any gameplay code that is not an OpenGL call.
   *
   * @param timestep  The amount of time (in seconds) since the last frame.
   */
  void update(float timestep) override;

  /**
   * Draws all this scene to the given SpriteBatch.
   *
   * @param batch     The SpriteBatch to draw with.
   */
  void render(const std::shared_ptr<cugl::SpriteBatch>& batch) override;

#pragma mark Collision Handling
  /**
   * Processes the start of a collision.
   *
   * @param  contact  The two bodies that collided.
   */
  void beginContact(b2Contact* contact);

  /**
   * Handles any modifications necessary before collision resolution.
   *
   * @param  contact  The two bodies that collided.
   * @param  contact  The collision manifold before contact.
   */
  void beforeSolve(b2Contact* contact, const b2Manifold* oldManifold);

  /**
   * The method called to update the camera in terms of the player position.
   *
   * @param timestep The amount of time (in seconds) since the last frame.
   */
  void updateCamera(float timestep);

  /**
   * Returns the network connection (as made by this scene).
   *
   * This value will be reset every time the scene is made active.
   *
   * @return the network connection (as made by this scene)
   */
  void setConnection(const std::shared_ptr<cugl::NetworkConnection>& network) {
    _network = network;
  }

  /**
   * Sets whether the player is host.
   *
   * We may need to have gameplay specific code for host.
   *
   * @param host  Whether the player is host.
   */
  void setHost(bool host) { _ishost = host; }

  /**
   * Checks that the network connection is still active.
   *
   * Even if you are not sending messages all that often, you need to be calling
   * this method regularly. This method is used to determine the current state
   * of the scene.
   *
   * @return true if the network connection is still active.
   */
  bool checkConnection();

  /**
   * Processes data sent over the network.
   *
   * Once connection is established, all data sent over the network consistes of
   * byte vectors. This function is a call back function to process that data.
   * Note that this function may be called *multiple times* per animation frame,
   * as the messages can come from several sources.
   *
   * @param data  The data received
   */
  void processData(const std::vector<uint8_t>& data);

  /**
   * Broadcasts the relevant network information to all clients and/or the host.
   */
  void sendNetworkInfo();

  /**
   * Broadcasts enemy being hit to the host.
   *
   * @param id the enemy that was hit
   * @param room_id the room the enemy is in
   */
  void sendEnemyHitNetworkInfo(int id, int room_id);

  /**
   * Updates the position of the player with the corresponding player_id in
   * the _players list.
   *
   * @param player_id The player id
   * @param pos_x The updated player x position
   * @param pos_y The updated player y position
   */
  void updatePlayerInfo(int player_id, float pos_x, float pos_y);

  /**
   * Updates the health and position of the enemy with the corresponding
   * enemy_id in the room with id enemy_room;
   *
   * @param enemy_id      The enemy id.
   * @param enemy_room    The room id the enemy is in.
   * @param enemy_health  The updated enemy health.
   * @param pos_x         The updated enemy x position.
   * @param pos_y         The updated enemy y position.
   */
  void updateEnemyInfo(int enemy_id, int enemy_room, int enemy_health,
                       float pos_x, float pos_y);

  /**
   * Returns true if the player quits the game.
   *
   * @return true if the player quits the game.
   */
  bool didQuit() const { return _quit; }

  /**
   * Disconnects this scene from the network controller.
   *
   * Technically, this method does not actually disconnect the network
   * controller. Since the network controller is a smart pointer, it is only
   * fully disconnected when ALL scenes have been disconnected.
   */
  void disconnect() { _network = nullptr; }
};

#endif /* SCENES_GAME_SCENE_H_ */
