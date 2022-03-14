// THIS IS A TEMPORARY FILE THAT SHOULD BE REPLACED USING JSON LOADING.

#ifndef MODELS_LEVEL_GEN_DEFAULT_ROOMS_H_
#define MODELS_LEVEL_GEN_DEFAULT_ROOMS_H_

#include "Room.h"

namespace default_rooms {

/** A basic structure that represents a room. */
struct RoomConfig {
  /** The size of the room. */
  cugl::Size size;
  /** The coordinates of the doors in the room. Ordered in counter-clockwise
   * order with right-most door first.*/
  std::vector<cugl::Vec2> doors;
  /** The source to the room scene2 node to copy. */
  std::string scene2_source;
};

// TODO: define size and door config using the source file.

/** Represents the default terminal room size and doors. */
const RoomConfig kTerminal = {
    cugl::Size(21.0f, 21.0f),
    std::vector<cugl::Vec2>{cugl::Vec2(20.0f, 10.0f), cugl::Vec2(10.0f, 20.0f),
                            cugl::Vec2(0.0f, 10.0f), cugl::Vec2(10.0f, 0.0f)},
    "rooms/terminal.json"};

/** Represents the default spawn room size and doors. */
const RoomConfig kSpawn = {
    cugl::Size(21.0f, 21.0f),
    std::vector<cugl::Vec2>{cugl::Vec2(20.0f, 10.0f), cugl::Vec2(10.0f, 20.0f),
                            cugl::Vec2(0.0f, 10.0f), cugl::Vec2(10.0f, 0.0f)},
    "rooms/spawn.json"};

/** Represents a standard room. */
const RoomConfig kStandard1 = {
    cugl::Size(21.0f, 21.0f),
    std::vector<cugl::Vec2>{cugl::Vec2(20.0f, 10.0f), cugl::Vec2(10.0f, 20.0f),
                            cugl::Vec2(0.0f, 10.0f), cugl::Vec2(10.0f, 0.0f)},
    "rooms/room-1-scene.json"};

/** Represents a standard room. */
const RoomConfig kStandard2 = {
    cugl::Size(21.0f, 21.0f),
    std::vector<cugl::Vec2>{cugl::Vec2(20.0f, 10.0f), cugl::Vec2(10.0f, 20.0f),
                            cugl::Vec2(0.0f, 10.0f), cugl::Vec2(10.0f, 0.0f)},
    "rooms/room-2-scene.json"};

/** RRepresents a standard rooms. */
const RoomConfig kStandard3 = {
    cugl::Size(21.0f, 21.0f),
    std::vector<cugl::Vec2>{cugl::Vec2(20.0f, 10.0f), cugl::Vec2(10.0f, 20.0f),
                            cugl::Vec2(0.0f, 10.0f), cugl::Vec2(10.0f, 0.0f)},
    "rooms/room-3-scene.json"};

/** A list of all the regular rooms. */
const std::vector<RoomConfig> kRegularRooms{kStandard1, kStandard2, kStandard3};

};  // namespace default_rooms

#endif  // MODELS_LEVEL_GEN_DEFAULT_ROOMS_H_