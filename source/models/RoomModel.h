#ifndef MODELS_ROOM_MODEL_H_
#define MODELS_ROOM_MODEL_H_

#include "./level_gen/RoomTypes.h"
#include "EnemyModel.h"

class RoomModel {
  /** This is the room type of the room. */
  RoomType _type;

  /** A reference to the scene2 node for the room. Has all the tiles and enemies
   * for the room. */
  std::shared_ptr<cugl::scene2::SceneNode> _node;

  /** A list of all the enemies inside of this room. */
  std::vector<std::shared_ptr<EnemyModel>> _enemies;

  /** A map between the door sensor id to the room id it points to. */
  std::unordered_map<std::string, int> _door_sensor_id_to_room_id;

  /** A map between the door sensor id and the position the player will teleport
   * to in the other room. */
  std::unordered_map<std::string, cugl::Vec2>
      _door_sensor_id_to_destination_pos;

  /** The key for the room. */
  int _key;

  /** The grid layout size for this room (i.e. the width and height of tiles) */
  cugl::Size _grid_size;

  /** The number of players required for activation. */
  int _num_players_required;

  /** The amount of energy to activate the room, if it is a terminal room.*/
  int _energy_to_activate;

  /** The amount of corrupted energy to activate the room for betrayers, if it
   * is a terminal room. */
  int _corrupted_energy_to_activate;

  /** The current energy count. */
  int _energy;

  /** The current corrupted energy count. */
  int _corrupted_energy;

 public:
  /**
   * Construct an empty RoomModel, please never use this. Instead use alloc().
   */
  RoomModel()
      : _num_players_required(-1),
        _key(-1),
        _type(RoomType::STANDARD),
        _energy_to_activate(100),
        _corrupted_energy_to_activate(100),
        _energy(0),
        _corrupted_energy(0) {}
  /** Destroy this RoomModel and all it's internal data. */
  ~RoomModel() { dispose(); }

  /**
   * Initialize the room model with the given scene2 node and name.
   *
   * @param node The scene2 node with all the tiles and enemies for the room.
   * @param key The key for the room.
   */
  bool init(const std::shared_ptr<cugl::scene2::SceneNode>& node, int key);

  /** Dispose of all the internal data in the room. */
  void dispose();

  /**
   * Allocate a new Room Model and return a smart pointer for it.
   *
   * @return A smart pointer of the instantiated RoomModel.
   */
  static std::shared_ptr<RoomModel> alloc(
      const std::shared_ptr<cugl::scene2::SceneNode>& node, int key) {
    std::shared_ptr<RoomModel> result = std::make_shared<RoomModel>();
    return (result->init(node, key) ? result : nullptr);
  }

  /**
   * Add a connection to another room using the door sensor ID that activates
   * it, the room ID that it goes to, and the position the player should
   * transport to when going to that room using this door sensor.
   *
   * @param door_sensor_id A string that represent the door sensor ID used.
   * @param room_id The ID of the room the door should go to.
   * @param destination The destination of teleporting the player.
   */
  void addConnection(std::string& door_sensor_id, int room_id,
                     cugl::Vec2& destination) {
    if (_door_sensor_id_to_room_id.find(door_sensor_id) ==
        _door_sensor_id_to_room_id.end()) {
      _door_sensor_id_to_room_id[door_sensor_id] = room_id;
      _door_sensor_id_to_destination_pos[door_sensor_id] = destination;
    }
  }

  /**
   * Get all the connections to other rooms from this room. Returns a map from
   * the door that connects the room to the room it connects to.
   *
   * @return An unordered map from the door sensor id to the room id.
   */
  std::unordered_map<std::string, int> getAllConnectedRooms() const {
    return _door_sensor_id_to_room_id;
  }

  /**
   * Get the Room ID a door sensor ID will take you to.
   *
   * @param door_sensor_id A string that represent the door sensor ID used.
   * @return The room ID.
   */
  int getRoomIdFromDoorSensorId(std::string& door_sensor_id) {
    if (_door_sensor_id_to_room_id.find(door_sensor_id) !=
        _door_sensor_id_to_room_id.end()) {
      return _door_sensor_id_to_room_id[door_sensor_id];
    }
    return -1;
  }

  /**
   * Get the position of the destination for the player when the given door
   * sensor ID is hit.
   *
   * @param door_sensor_id A string that represent the door sensor ID used.
   * @return The destination for the player.
   */
  cugl::Vec2 getPosOfDestinationDoor(std::string& door_sensor_id) {
    if (_door_sensor_id_to_destination_pos.find(door_sensor_id) !=
        _door_sensor_id_to_destination_pos.end()) {
      return _door_sensor_id_to_destination_pos[door_sensor_id];
    }
    return cugl::Vec2::ZERO;
  }

  /**
   * Set the visibility of the room model node.
   * @param val Visible or not.
   */
  void setVisible(bool val) { _node->setVisible(val); }

  /**
   * Get The scene2 node for the RoomModel.
   *
   * @return The scene2 node.
   */
  std::shared_ptr<cugl::scene2::SceneNode> getNode() const { return _node; }

  /**
   * Get the key of this room, for debugging purposes.
   *
   * @return The key of the room.
   * */
  int getKey() const { return _key; }

  /**
   * The grid layout size of the room (i.e. how many tiles in x and y).
   *
   * @return The grid layout size.
   */
  cugl::Size getGridSize() const { return _grid_size; }

  /**
   * Set the enemies for this room.
   *
   * @param enemies The enemies.
   */
  void setEnemies(std::vector<std::shared_ptr<EnemyModel>>& enemies) {
    _enemies = enemies;
  }

  /**
   * Get the enemies for this room.
   * @return The enemies.
   */
  std::vector<std::shared_ptr<EnemyModel>>& getEnemies() { return _enemies; }

  /**
   * Set the room type.
   * @param type The room type.
   */
  void setType(RoomType type) { _type = type; }

  /**
   * Get the room type.
   * @return The room type.
   */
  int getType() { return _type; }

  /**
   * Set the number of players required for terminal activation.
   * @param num The number of players required.
   */
  void setNumPlayersRequired(int num) { _num_players_required = num; }

  /**
   * Get the number of players required for terminal activation.
   * @return The number of players required.
   */
  int getNumPlayersRequired() { return _num_players_required; }

  /**
   * Get the energy level of this terminal room.
   * @return The energy level.
   */
  int getEnergy() { return _energy; }
  /**
   * Set the energy level of this terminal room
   * @param energy the amount of energy.
   */
  void setEnergy(int energy) { _energy = energy; }

  /**
   * Get the corrupted energy level of this terminal room.
   * @return The corrupted energy level.
   */
  int getCorruptedEnergy() { return _corrupted_energy; }
  /**
   * Set the corrupted energy level of this terminal room
   * @param energy the amount of corrupted energy.
   */
  void setCorruptedEnergy(int energy) { _corrupted_energy = energy; }

  /**
   * Get the amount of energy needed to activate the terminal.
   * @return The amount of energy needed to activate the terminal.
   */
  int getEnergyToActivate() { return _energy_to_activate; }
  /**
   * Set the energy level needed to activate this terminal room
   * @param energy the amount of energy needed to activate this terminal.
   */
  void setEnergyToActivate(int energy) { _energy_to_activate = energy; }

  /**
   * Get the amount of corrupted energy needed to activate the terminal.
   * @return The amount of corrupted energy needed to activate the terminal.
   */
  int getCorruptedEnergyToActivate() { return _corrupted_energy_to_activate; }
  /**
   * Set the energy level needed to activate this terminal room
   * @param energy the amount of energy needed to activate this terminal.
   */
  void setCorruptedEnergyToActivate(int energy) {
    _corrupted_energy_to_activate = energy;
  }
};

#endif  // MODELS_ROOM_MODEL_H_
