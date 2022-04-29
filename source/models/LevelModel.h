#ifndef MODELS_LEVEL_MODEL_H_
#define MODELS_LEVEL_MODEL_H_

#include "Player.h"
#include "RoomModel.h"

class LevelModel {
  /** A reference to the room the player is currently in. */
  std::shared_ptr<RoomModel> _current_room;

  /** A map from a room id to the room it corresponds to, useful for switching
   * between rooms when passing through a door. */
  std::unordered_map<int, std::shared_ptr<RoomModel>> _rooms;

  /** A reference to the spawn room for the level. */
  std::shared_ptr<RoomModel> _spawn_room;

  /** A reference to the palyer model. */
  std::shared_ptr<Player> _player;

 public:
  /** Construct an empty LevelModel, please never use this. Instead, use
   * alloc(). */
  LevelModel() : _current_room(nullptr), _player(nullptr) {}
  /** Destruct this LevelModel. */
  ~LevelModel() { dispose(); }

  /**
   * Initialize the LevelModel.
   *
   * @return If successfully initialized.
   */
  bool init();

  /**  Dispose of the LevelModel and all its local data. */
  void dispose();

  /**
   * Allocate a new LevelModel and return a smart pointer for it.
   *
   * @return Smart pointer with the allocated LevelModel, or nullptr if not
   * successfully initialized.
   */
  static std::shared_ptr<LevelModel> alloc() {
    std::shared_ptr<LevelModel> result = std::make_shared<LevelModel>();
    return (result->init() ? result : nullptr);
  }

  /**
   * Add a Room with a unique ID to the level.
   *
   * @param unique_id An int key to access the room later.
   * @param room The smart pointer referencing the room to be added.
   */
  void addRoom(int unique_id, const std::shared_ptr<RoomModel>& room) {
    // If Room Id doesn't already exists.
    if (_rooms.find(unique_id) == _rooms.end()) _rooms[unique_id] = room;
  }

  /**
   * Set the room the player is currently on. Useful for updating only enemies,
   * objects of one room, and to cull all other room.s
   *
   * @param room_id The unique ID of the room to be set to.
   */
  void setCurrentRoom(int room_id) {
    if (_rooms.find(room_id) == _rooms.end()) return;
    _current_room = _rooms[room_id];
  }

  /**
   * Get the room the player is currently on.
   *
   * @return The room the player is currently on.
   */
  std::shared_ptr<RoomModel> getCurrentRoom() const { return _current_room; }

  /**
   * Set the spawn room for the level.
   * @param room The pointer to the room.
   */
  void setSpawnRoom(const std::shared_ptr<RoomModel>& room) {
    _spawn_room = room;
  }

  /**
   * Get the spawn room for the level.
   * @return The spawn room for the level.
   */
  std::shared_ptr<RoomModel> getSpawnRoom() const { return _spawn_room; }

  /**
   * Get the room given a room key.
   *
   * @param room_id The unique key that corresponds to this room.
   * @return A room that corresponds to the given key. nullptr if room doesn't
   *exist.
   */
  std::shared_ptr<RoomModel> getRoom(int room_id) {
    if (_rooms.find(room_id) == _rooms.end()) return nullptr;
    return _rooms[room_id];
  }

  /**
   * Get the map of all the unique room IDs to all the rooms in the level.
   *
   * @return An unordered map that maps room IDs to room objects.
   */
  std::unordered_map<int, std::shared_ptr<RoomModel>> getRooms() const {
    return _rooms;
  }

  /**
   * Set the player model in the level model.
   *
   * @param player The player model.
   */
  void setPlayer(const std::shared_ptr<Player>& player) { _player = player; }

  /**
   * Get the player model.
   *
   * @return The player model.
   */
  std::shared_ptr<Player> getPlayer() const { return _player; }
};

#endif  // MODELS_LEVEL_MODEL_H_
