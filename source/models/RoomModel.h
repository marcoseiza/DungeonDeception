#ifndef MODELS_ROOM_MODEL_H_
#define MODELS_ROOM_MODEL_H_

#include "Grunt.h"

class RoomModel {
  /** A reference to the scene2 node for the room. Has all the tiles and enemies
   * for the room. */
  std::shared_ptr<cugl::scene2::SceneNode> _node;

  /** A list of all the enemies inside of this room. */
  std::vector<std::shared_ptr<Grunt>> _enemies;

  /** A map between the door sensor id to the room id it points to. */
  std::unordered_map<std::string, std::string> _door_sensor_id_to_room_id;

  /** A map between the door sensor id and the position the player will teleport
   * to in the other room. */
  std::unordered_map<std::string, cugl::Vec2>
      _door_sensor_id_to_destination_pos;

  /** The name of the room, for debugging purposes. */
  std::string _name;

  /** The grid layout size for this room (i.e. the width and height of tiles) */
  cugl::Size _grid_size;

 public:
  /**
   * Construct an empty RoomModel, please never use this. Instead use alloc().
   */
  RoomModel() {}
  /** Destroy this RoomModel and all it's internal data. */
  ~RoomModel() { dispose(); }

  /**
   * Initialize the room model with the given scene2 node and name.
   *
   * @param node The scene2 node with all the tiles and enemies for the room.
   * @param name The name of the room, for debugging purposes.
   */
  bool init(const std::shared_ptr<cugl::scene2::SceneNode>& node,
            std::string& name);

  /** Dispose of all the internal data in the room. */
  void dispose();

  /**
   * Allocate a new Room Model and return a smart pointer for it.
   *
   * @return A smart pointer of the instantiated RoomModel.
   */
  static std::shared_ptr<RoomModel> alloc(
      const std::shared_ptr<cugl::scene2::SceneNode>& node, std::string& name) {
    std::shared_ptr<RoomModel> result = std::make_shared<RoomModel>();
    return (result->init(node, name) ? result : nullptr);
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
  void addConnection(std::string& door_sensor_id, std::string& room_id,
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
  std::unordered_map<std::string, std::string> getAllConnectedRooms() const {
    return _door_sensor_id_to_room_id;
  }

  /**
   * Get the Room ID a door sensor ID will take you to.
   *
   * @param door_sensor_id A string that represent the door sensor ID used.
   * @return The room ID.
   */
  std::string getRoomIdFromDoorSensorId(std::string& door_sensor_id) {
    if (_door_sensor_id_to_room_id.find(door_sensor_id) !=
        _door_sensor_id_to_room_id.end()) {
      return _door_sensor_id_to_room_id[door_sensor_id];
    }
    return "";
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
   * Get the Name of this room, for debugging purposes.
   *
   * @return The name of the room.
   * */
  std::string getName() const { return _name; }

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
  void setEnemies(std::vector<std::shared_ptr<Grunt>>& enemies) {
    _enemies = enemies;
  }

  /**
   * Get the enemies for this room.
   * @return The enemies.
   */
  std::vector<std::shared_ptr<Grunt>>& getEnemies() { return _enemies; }
};

#endif  // MODELS_ROOM_MODEL_H_