#ifndef NETWORK_STRUCTS_SERIALIZABLE
#define NETWORK_STRUCTS_SERIALIZABLE

#include <cugl/net/CUNetworkSerializer.h>

#include <unordered_map>

namespace cugl {

/**
 * This is a generic class used to create serializable structs to send over the
 * network.
 *
 * See EnemyInfo in "EnemyStructs.h" for a good example.
 *
 * MAKE SURE TO ALSO ADD AN INITIALIZING FUNCTION IN
 * CustomNetworkDeserializer::CustomNetworkDeserializer()
 */
struct Serializable {
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
  virtual void serialize(NetworkSerializer* serializer) {}

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
  virtual void deserialize(NetworkDeserializer* deserializer) {}

  /**
   * This method returns a unique key to the struct.
   * @return The unique key.
   */
  virtual Uint32 key() = 0;
};

}  // namespace cugl

#endif  // NETWORK_STRUCTS_SERIALIZABLE
