#ifndef SCENES_GAME_SCENE_H_
#define SCENES_GAME_SCENE_H_
#include <box2d/b2_world_callbacks.h>
#include <cugl/cugl.h>

#include "../controllers/Controller.h"
#include "../controllers/InputController.h"
#include "../controllers/LevelController.h"
#include "../controllers/ParticleController.h"
#include "../controllers/PlayerController.h"
#include "../controllers/SoundController.h"
#include "../controllers/TerminalController.h"
#include "../controllers/enemies/GruntController.h"
#include "../controllers/enemies/ShotgunnerController.h"
#include "../controllers/enemies/TankController.h"
#include "../controllers/enemies/TurtleController.h"
#include "../generators/LevelGenerator.h"
#include "../models/Player.h"
#include "../network/NetworkController.h"
#include "SettingsScene.h"

class GameScene : public cugl::Scene2 {
 public:
  /** The game state. */
  enum State {
    /** Default state. */
    NONE,
    /** The game is running */
    RUN,
    /** User wants to leave. */
    LEAVE,
    /** The game is finished */
    DONE
  };

 protected:
  /** The asset manager for loading. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** The animated health bar */
  std::shared_ptr<cugl::scene2::ProgressBar> _health_bar;

  /** The animated energy bar */
  std::shared_ptr<cugl::scene2::ProgressBar> _energy_bar;

  /** Reference to the physics root of the scene graph. */
  std::shared_ptr<cugl::scene2::SceneNode> _world_node;

  /** Reference to the particle root in world of the scene graph. */
  std::shared_ptr<cugl::scene2::SceneNode> _particle_world;
  /** Reference to the particle root in screen of the scene graph. */
  std::shared_ptr<cugl::scene2::SceneNode> _particle_screen;

  /** Reference to the debug root of the scene graph. */
  std::shared_ptr<cugl::scene2::SceneNode> _debug_node;

  /** Reference to the role screen scene graph. */
  std::shared_ptr<cugl::scene2::Button> _role_layer;

  /** Reference to the cloud layer scene graph. */
  std::shared_ptr<cugl::scene2::SceneNode> _cloud_layer;

  /** The Box2d world */
  std::shared_ptr<cugl::physics2::ObstacleWorld> _world;

  /** The sound controller that starts, pauses and keeps track of sounds. */
  std::shared_ptr<SoundController> _sound_controller;
  /** The player controller for the game*/
  std::shared_ptr<PlayerController> _player_controller;
  /** The grunt controller for the game. */
  std::shared_ptr<GruntController> _grunt_controller;
  /** The shotgunner controller for the game. */
  std::shared_ptr<ShotgunnerController> _shotgunner_controller;
  /** The tank controller for the game. */
  std::shared_ptr<TankController> _tank_controller;
  /** The turtle controller for the game. */
  std::shared_ptr<TurtleController> _turtle_controller;

  /** A reference to the particle controller. */
  std::shared_ptr<ParticleController> _particle_controller;

  /** The level controller for the game*/
  std::shared_ptr<LevelController> _level_controller;
  /** The terminal controller for voting in the game. */
  std::shared_ptr<TerminalController> _terminal_controller;

  /** Reference to the settings scene for exiting game. */
  std::shared_ptr<SettingsScene> _settings_scene;

  /** A reference to the scene2 map for rendering. */
  std::shared_ptr<cugl::scene2::SceneNode> _map;

  /** The serializer used to serialize complex data to send through the network.
   */
  cugl::NetworkSerializer _serializer;

  /** The deserializer used to deserialize complex data sent through the
   * network. */
  cugl::NetworkDeserializer _deserializer;

  /** Whether this player is the host. */
  bool _ishost;

  /** Whether this player is a betrayer. */
  bool _is_betrayer;

  /** The state of the game. */
  State _state;

  /** The number of terminals in the world. */
  int _num_terminals;

  /** The number of terminals activated in the world. */
  int _num_terminals_activated;

  /** The number of terminals corrupted in the world. */
  int _num_terminals_corrupted;

  /** List of blocked X's. 6 for maximum number of runners .*/
  std::array<std::shared_ptr<cugl::scene2::SceneNode>, 6> _block_x_nodes;

  /** The display name of my player. */
  std::string _display_name;

  /** Timestamp so unimportant enemy info isn't sent. */
  cugl::Timestamp _time_of_last_enemy_other_info_update;
  /** A list of enemy IDs to die. */
  std::vector<int> _dead_enemy_cache;

  /** Timestamp so unimportant player info isn't sent every tick. */
  cugl::Timestamp _time_of_last_player_other_info_update;
  /** If the has sent play basic_info to all clients. */
  bool _has_sent_player_basic_info;

  /** Energy particle for when enemies die. */
  ParticleProps _energy_particle;
  /** Energy particle for when player deposits energy. */
  ParticleProps _deposit_particle_regular, _deposit_particle_corrupted;

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
   * @param assets        The (loaded) assets for this game mode.
   * @param level_gen     The generated level.
   * @param map           The world map.
   * @param is_betrayer   True if the game is being played by a betrayer.
   * @param display_name  Name the player input in lobby.
   * @param color_ids     A map from player id to color id.

   *
   * @return true if the controller is initialized properly, false otherwise.
   */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets,
            const std::shared_ptr<level_gen::LevelGenerator>& level_gen,
            const std::shared_ptr<cugl::scene2::SceneNode>& map,
            bool is_betrayer, std::string display_name,
            std::unordered_map<int, int> color_ids);

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
   * Check if the betrayers have won.
   *
   * @return true if at least half of the terminals have been activated.
   */
  bool checkBetrayerWin();

  /**
   * The method called to update the game mode.
   * This method contains any gameplay code that is not an OpenGL call.
   *
   * @param timestep  The amount of time (in seconds) since the last frame.
   */
  void update(float timestep) override;

  /**
   * This method serves as a helper to updating all the enemies
   *
   * @param timestep The amount of time (in seconds) since the last frame.
   * @param room The room to update the enemies in.
   */
  void updateEnemies(float timestep, std::shared_ptr<RoomModel> room);

  /**
   * Returns an unordered set of all the room ids players are in.
   */
  std::unordered_set<int> getRoomIdsWithPlayers() {
    std::unordered_set<int> room_ids_with_players;
    for (std::shared_ptr<Player> player : _player_controller->getPlayerList()) {
      int room_id = player->getRoomId();
      if (room_id != -1) {
        room_ids_with_players.emplace(room_id);
      }
    }
    return room_ids_with_players;
  }

  /**
   * Returns an unordered set of all the room ids players are in, and adjacent
   * rooms.
   */
  std::unordered_set<int> getAdjacentRoomIdsWithPlayers() {
    std::unordered_set<int> room_ids_with_players = getRoomIdsWithPlayers();
    std::unordered_set<int> all_enemy_update_rooms;
    for (auto room_id_to_update : room_ids_with_players) {
      // Add the player room.
      if (room_id_to_update != -1) {
        all_enemy_update_rooms.emplace(room_id_to_update);
      }
      // Add the adjacent player rooms.
      auto room =
          _level_controller->getLevelModel()->getRoom(room_id_to_update);
      auto rooms = room->getAllConnectedRooms();
      for (auto room_map : rooms) {
        all_enemy_update_rooms.emplace(room_map.second);
      }
    }
    return all_enemy_update_rooms;
  }

  /**
   * Returns an unordered set of all the player adjacent room ids without the
   * player rooms.
   */
  std::unordered_set<int> getAdjacentRoomIdsWithoutPlayers() {
    std::unordered_set<int> room_ids_with_players = getRoomIdsWithPlayers();
    std::unordered_set<int> all_enemy_update_rooms;
    for (auto room_id_to_update : room_ids_with_players) {
      // Add the adjacent player rooms.
      auto room =
          _level_controller->getLevelModel()->getRoom(room_id_to_update);
      auto rooms = room->getAllConnectedRooms();
      for (auto room_map : rooms) {
        if (room_ids_with_players.count(room_map.second) == 0) {
          all_enemy_update_rooms.emplace(room_map.second);
        }
      }
    }
    return all_enemy_update_rooms;
  }

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
   * The method called to update the camera in terms of the player position.
   *
   * @param timestep The amount of time (in seconds) since the last frame.
   */
  void updateCamera(float timestep);

#pragma mark Networking

  /**
   * Sets the map SceneNode.
   */
  void setMap(const std::shared_ptr<cugl::scene2::SceneNode>& map) {
    _map = map;
  }

  /**
   * Sets whether the player is host.
   *
   * We may need to have gameplay specific code for host.
   *
   * @param host  Whether the player is host.
   */
  void setHost(bool host) {
    _ishost = host;
    NetworkController::get()->setIsHost(host);
  }

  /**
   * Sets whether the player is a betrayer or cooperator.
   *
   * @param betrayer  Whether the player is a betrayer.
   */
  void setBetrayer(bool betrayer) { _is_betrayer = betrayer; }

  /**
   * Processes data sent over the network.
   *
   * Once connection is established, all data sent over the network consistes of
   * byte vectors. This function is a call back function to process that data.
   * Note that this function may be called *multiple times* per animation frame,
   * as the messages can come from several sources.
   *
   * @param code The message code
   * @param msg The deserialized message
   */
  void processData(const Sint32& code,
                   const cugl::CustomNetworkDeserializer::CustomMessage& msg);

  /**
   * Returns the network connection (as made by this scene).
   *
   * This value will be reset every time the scene is made active.
   *
   * @return the network connection (as made by this scene)
   */
  void setConnection(const std::shared_ptr<cugl::NetworkConnection>& network);

  /**
   * Broadcasts the relevant network information to all clients and/or the host.
   */
  void sendNetworkInfo();

  /**
   * Broadcasts the relevant network information if a host.
   */
  void sendNetworkInfoHost();

  /**
   * Broadcasts the relevant network information if a client.
   */
  void sendNetworkInfoClient();

  /**
   * Broadcasts enemy being hit to the host.
   *
   * @param player_id the player that hit the enemy
   * @param enemy_id the enemy that was hit
   * @param amount the amount of damage taken
   */
  void sendEnemyHitNetworkInfo(int player_id, int enemy_id, float amount = 20);

  /**
   * Broadcast a player being targeted by the runner block player ability.
   *
   * @param runner_id The runner who hit the button.
   * @param target_player_id The player being targeted.
   */
  void sendBetrayalTargetInfo(int runner_id, int target_player_id);

  /**
   * Broadcast a player being disabled by a betrayer ability.
   *
   * @param runner_id The runner who hit the button.
   * @param target_player_id The player being targeted.
   */
  void sendDisablePlayerInfo(int runner_id, int target_player_id);

  /**
   *
   * Send to host a player being corrupted by the betrayer corrupt player
   * ability.
   *
   * @param corrupt_player_id The player being corrupted.
   */
  void sendBetrayalCorruptInfo(int corrupt_player_id);

  /**
   * Updates the position of the player with the corresponding player_id in
   * the _players list.
   *
   * @param player_id The player id
   * @param room_id   The room id
   * @param pos_x The updated player x position
   * @param pos_y The updated player y position
   */
  void updatePlayerInfo(int player_id, int room_id, float pos_x, float pos_y);

  /**
   * Returns the game state
   * @return The game state
   */
  State getState() const { return _state; }

  /**
   * Disconnects this scene from the network controller.
   *
   * Technically, this method does not actually disconnect the network
   * controller. Since the network controller is a smart pointer, it is only
   * fully disconnected when ALL scenes have been disconnected.
   */
  void disconnect() { NetworkController::get()->disconnect(); }
};

#endif /* SCENES_GAME_SCENE_H_ */
