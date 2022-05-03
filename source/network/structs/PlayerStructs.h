#ifndef NETWORK_STRUCTS_PLAYER_STRUCTS_H_
#define NETWORK_STRUCTS_PLAYER_STRUCTS_H_

#include <cugl/math/cu_math.h>

#include "../NetworkCodes.h"
#include "Serializable.h"

#pragma mark -
#pragma mark PlayerInfo

namespace cugl {

struct PlayerInfo : public Serializable {
  /** The unique key for the struct. Must be static. */
  const static Uint32 Key;

  /** The ID of the player. */
  int player_id;
  /** The position of the player. */
  cugl::Vec2 pos;
  /** The room id the player is in. */
  int room_id;

  /**
   * Alloc a new serializable object
   * @return The shared pointer with the new object.
   */
  static std::shared_ptr<PlayerInfo> alloc() {
    return std::make_shared<PlayerInfo>();
  }

  /**
   * This method serializes the class into the given serializer.
   *
   * Override this method when creating a new serializable structs and write all
   * the properties of the struct into the serializer.
   *
   * THE ORDER IN WHICH YOU WRITE THEM IS IMPORTANT
   *
   * When you desserialize, you will have to read in the same order you wrote.
   *
   * @param serializer The network serializer.
   */
  void serialize(cugl::NetworkSerializer* serializer) override;

  /**
   * This method deserializes the given the deserializer.
   *
   * Override this method when creating a new serializable struct and read all
   * the properties of the struct from the deserializer.
   *
   * THE ORDER IN WHICH YOU READ THEM IS IMPORTANT
   *
   * Make sure you read in the same order you wrote the properties in the
   * serializer.
   *
   * @param deserializer
   */
  void deserialize(cugl::NetworkDeserializer* deserializer) override;

  /**
   * This method returns a unique key to the struct.
   * @return The unique key.
   */
  virtual Uint32 key() override { return Key; }
};

#pragma mark -
#pragma mark PlayerOtherInfo

struct PlayerOtherInfo : public Serializable {
  /** The unique key for the struct. Must be static. */
  const static Uint32 Key;

  /** The ID of the player. */
  int player_id;
  /** The amount of energy this player has. */
  int energy;
  /** The amount of corruption this player has. */
  int corruption;

  /**
   * Alloc a new serializable object
   * @return The shared pointer with the new object.
   */
  static std::shared_ptr<PlayerOtherInfo> alloc() {
    return std::make_shared<PlayerOtherInfo>();
  }

  /**
   * This method serializes the class into the given serializer.
   *
   * Override this method when creating a new serializable structs and write all
   * the properties of the struct into the serializer.
   *
   * THE ORDER IN WHICH YOU WRITE THEM IS IMPORTANT
   *
   * When you desserialize, you will have to read in the same order you wrote.
   *
   * @param serializer The network serializer.
   */
  void serialize(cugl::NetworkSerializer* serializer) override;

  /**
   * This method deserializes the given the deserializer.
   *
   * Override this method when creating a new serializable struct and read all
   * the properties of the struct from the deserializer.
   *
   * THE ORDER IN WHICH YOU READ THEM IS IMPORTANT
   *
   * Make sure you read in the same order you wrote the properties in the
   * serializer.
   *
   * @param deserializer
   */
  void deserialize(cugl::NetworkDeserializer* deserializer) override;

  /**
   * This method returns a unique key to the struct.
   * @return The unique key.
   */
  virtual Uint32 key() override { return Key; }
};

#pragma mark -
#pragma mark BasicPlayerInfo

struct BasicPlayerInfo : public Serializable {
  /** The unique key for the struct. Must be static. */
  const static Uint32 Key;

  /** The ID of the player. */
  int player_id;
  /** The name to display above the player. */
  std::string name;
  /** If the player is a betrayer. */
  bool betrayer;

  /**
   * Alloc a new serializable object
   * @return The shared pointer with the new object.
   */
  static std::shared_ptr<BasicPlayerInfo> alloc() {
    return std::make_shared<BasicPlayerInfo>();
  }

  /**
   * This method serializes the class into the given serializer.
   *
   * Override this method when creating a new serializable structs and write all
   * the properties of the struct into the serializer.
   *
   * THE ORDER IN WHICH YOU WRITE THEM IS IMPORTANT
   *
   * When you desserialize, you will have to read in the same order you wrote.
   *
   * @param serializer The network serializer.
   */
  void serialize(cugl::NetworkSerializer* serializer) override;

  /**
   * This method deserializes the given the deserializer.
   *
   * Override this method when creating a new serializable struct and read all
   * the properties of the struct from the deserializer.
   *
   * THE ORDER IN WHICH YOU READ THEM IS IMPORTANT
   *
   * Make sure you read in the same order you wrote the properties in the
   * serializer.
   *
   * @param deserializer
   */
  void deserialize(cugl::NetworkDeserializer* deserializer) override;

  /**
   * This method returns a unique key to the struct.
   * @return The unique key.
   */
  virtual Uint32 key() override { return Key; }
};

}  // namespace cugl
#endif  // NETWORK_STRUCTS_PLAYER_STRUCTS_H_