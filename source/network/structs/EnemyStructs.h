#ifndef NETWORK_STRUCTS_ENEMY_STRUCTS_H_
#define NETWORK_STRUCTS_ENEMY_STRUCTS_H_

#include <cugl/math/cu_math.h>

#include "../NetworkCodes.h"
#include "Serializable.h"

namespace cugl {

#pragma mark -
#pragma mark EnemyInfo

struct EnemyInfo : public Serializable {
  /** The unique key for the struct. Must be static. */
  const static Uint32 Key;

  /** The ID of the enemy. */
  int enemy_id;
  /** The position of the enemy. */
  cugl::Vec2 pos;
  /** If the enemy has a target. */
  bool has_target;
  /** Where the photon target should spawn. */
  cugl::Vec2 target;

  /**
   * Alloc a new serializable object
   * @return The shared pointer with the new object.
   */
  static std::shared_ptr<EnemyInfo> alloc() {
    return std::make_shared<EnemyInfo>();
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
#pragma mark EnemyOtherInfo

struct EnemyOtherInfo : public Serializable {
  /** The unique key for the struct. Must be static. */
  const static Uint32 Key;

  /** The ID of the enemy. */
  int enemy_id;
  /** The amount of health the enemy has. */
  int health;

  /**
   * Alloc a new serializable object
   * @return The shared pointer with the new object.
   */
  static std::shared_ptr<EnemyOtherInfo> alloc() {
    return std::make_shared<EnemyOtherInfo>();
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
#pragma mark EnemyHitInfo

struct EnemyHitInfo : public Serializable {
  /** The unique key for the struct. Must be static. */
  const static Uint32 Key;

  /** The ID of the enemy. */
  int enemy_id;
  /** The ID of the player that hit the enemy. */
  int player_id;
  /** The amount of health the enemy has been hit by. */
  int amount;
  /** The direction of the hit, for knockback. */
  int direction;

  /**
   * Alloc a new serializable object
   * @return The shared pointer with the new object.
   */
  static std::shared_ptr<EnemyHitInfo> alloc() {
    return std::make_shared<EnemyHitInfo>();
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

#endif  // NETWORK_STRUCTS_ENEMY_STRUCTS_H_
