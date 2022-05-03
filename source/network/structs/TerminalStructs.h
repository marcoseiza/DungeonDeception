#ifndef NETWORK_STRUCTS_TERMINAL_STRUCTS_H_
#define NETWORK_STRUCTS_TERMINAL_STRUCTS_H_

#include <cugl/math/cu_math.h>

#include "../NetworkCodes.h"
#include "Serializable.h"

namespace cugl {

#pragma mark -
#pragma mark TerminalDeposit

struct TerminalDeposit : public Serializable {
  /** The unique key for the struct. Must be static. */
  const static Uint32 Key;

  /** The ID of the player. */
  int player_id;
  /** The terminal room id. */
  int room_id;

  /**
   * Alloc a new serializable object
   * @return The shared pointer with the new object.
   */
  static std::shared_ptr<TerminalDeposit> alloc() {
    return std::make_shared<TerminalDeposit>();
  }

  /**
   * This method serializes the class into the given serializer.
   * @param serializer The network serializer.
   */
  void serialize(cugl::NetworkSerializer* serializer) override;

  /**
   * This method deserializes the given the deserializer.
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
#pragma mark TerminalUpdate

struct TerminalUpdate : public Serializable {
  /** The unique key for the struct. Must be static. */
  const static Uint32 Key;

  /** The ID of the player. */
  int player_id;
  /** The amount of energy for the player. */
  int player_energy;
  /** The amount of corrupted energy for the player. */
  int player_corrupted_energy;
  /** The ID of the terminal room. */
  int room_id;
  /** The amount of energy for the room. */
  int room_energy;
  /** The amount of corrupted energy for the room. */
  int room_corrupted_energy;

  /**
   * Alloc a new serializable object
   * @return The shared pointer with the new object.
   */
  static std::shared_ptr<TerminalUpdate> alloc() {
    return std::make_shared<TerminalUpdate>();
  }

  /**
   * This method serializes the class into the given serializer.
   * @param serializer The network serializer.
   */
  void serialize(cugl::NetworkSerializer* serializer) override;

  /**
   * This method deserializes the given the deserializer.
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
#endif  // NETWORK_STRUCTS_TERMINAL_STRUCTS_H_