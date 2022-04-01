#ifndef MODELS_LEVEL_GEN_ROOM_TYPES_H_
#define MODELS_LEVEL_GEN_ROOM_TYPES_H_

/** RoomType is used by the level generator to define when the room should be
 * spawned. It is also used by the game loop for specific game interactions.  */
enum RoomType {
  /** Represents a terminal room*/
  TERMINAL,
  /** Represents a spawn room. */
  SPAWN,
  /** Represents any standard type of room. */
  STANDARD
};

#endif  // MODELS_LEVEL_GEN_ROOM_TYPES_H_