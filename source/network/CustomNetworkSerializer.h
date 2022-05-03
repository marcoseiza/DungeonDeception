#ifndef NETWORK_CUSTOM_NETWORK_SERIALIZER_H_
#define NETWORK_CUSTOM_NETWORK_SERIALIZER_H_

#include <cugl/net/CUNetworkSerializer.h>

#include "structs/EnemyStructs.h"
#include "structs/PlayerStructs.h"
#include "structs/Serializable.h"
#include "structs/TerminalStructs.h"

namespace cugl {
#pragma mark -
#pragma mark CustomNetworkSerializer
/**
 * A class to serialize complex data into a byte array.
 *
 * The class {@link NetworkConnection} is only capable of transmitting byte
 * arrays. You should use this class to construct a byte array for a single
 * message so that you can transmit it.
 *
 * This class is capable of serializing the following data types:
 *  - Floats
 *  - Doubles
 *  - 32 Bit Signed + Unsigned Integers
 *  - 64 Bit Signed + Unsigned Integers
 *  - Strings (see note below)
 *  - JsonValue (the cugl JSON class)
 *  - Vectors of all above types
 *
 * You should deserialize all of these with the {@link NetworkDeserializer}.
 *
 * Note that if a char* (not a C++ string) is written, it will be deserialized
 * as a std::string. The same applies to vectors of char*.
 */
class CustomNetworkSerializer : public NetworkSerializer {
 public:
  /**
   * Creates a new CustomNetwork Serializer on the stack.
   *
   * Network serializers do not have any nontrivial state and so it is
   * unnecessary to use an init method. However, we do include a static {@link
   * #alloc} method for creating shared pointers.
   */
  CustomNetworkSerializer() {}

  /**
   * Returns a newly created Custom Network Serializer.
   *
   * This method is solely include for convenience purposes.
   *
   * @return a newly created Custom Network Serializer.
   */
  static std::shared_ptr<CustomNetworkSerializer> alloc() {
    return std::make_shared<CustomNetworkSerializer>();
  }

  /**
   * Writes a single Serializable object.
   *
   * Values will be deserialized on other machines in the same order they were
   * written in. Pass the result of {@link #serialize} to the {@link
   * NetworkConnection} to send all values buffered up to this point.
   *
   * @param v The value to write
   */
  void writeSerializable(const std::shared_ptr<Serializable>& v);

  /**
   * Writes a vector of Serializable objects.
   *
   * Values will be deserialized on other machines in the same order they were
   * written in. Pass the result of {@link #serialize} to the {@link
   * NetworkConnection} to send all values buffered up to this point.
   *
   * @param v The value to write
   */
  void writeSerializableVector(std::vector<std::shared_ptr<Serializable>> v);
};

#pragma mark -
#pragma mark CustomNetworkDeserializer
/**
 * A class to deserializes byte arrays back into the original complex data.
 *
 * This class only handles messages serialized using {@link NetworkSerializer}.
 * You should use {@link NetworkType} to guide your deserialization process.
 */
class CustomNetworkDeserializer : public NetworkDeserializer {
 private:
  std::unordered_map<Uint32, std::function<std::shared_ptr<Serializable>()>>
      _factory;

 public:
  /**
   * Variant of possible messages to receive.
   *
   * To understand how to use variants, see this tutorial:
   *
   * https://riptutorial.com/cplusplus/example/18604/basic-std--variant-use
   *
   * This type is to be used with the {@link #read()} method. The variant
   * monostate represents no more content.
   */
  typedef std::variant<
      std::monostate, bool, float, double, Uint32, Uint64, Sint32, Sint64,
      std::string, std::shared_ptr<JsonValue>, std::shared_ptr<Serializable>,
      std::vector<bool>, std::vector<float>, std::vector<double>,
      std::vector<Uint32>, std::vector<Uint64>, std::vector<Sint32>,
      std::vector<Sint64>, std::vector<std::string>,
      std::vector<std::shared_ptr<JsonValue>>,
      std::vector<std::shared_ptr<Serializable>>>
      CustomMessage;

  /**
   * Creates a new Network Deserializer on the stack.
   *
   * Network deserializers do not have any nontrivial state and so it is
   * unnecessary to use an init method. However, we do include a static {@link
   * #alloc} method for creating shared pointers.
   */
  CustomNetworkDeserializer();

  /**
   * Returns a newly created Network Deserializer.
   *
   * This method is solely include for convenience purposes.
   *
   * @return a newly created Network Deserializer.
   */
  static std::shared_ptr<CustomNetworkDeserializer> alloc() {
    return std::make_shared<CustomNetworkDeserializer>();
  }

  /**
   * Reads the next unreturned value or vector from the currently loaded byte
   * vector.
   *
   * A byte vector should be loaded with the {@link #receive} method. If
   * nothing is loaded, this will return the monostate. This method also
   * advances the read position. If the end of the vector is reached, this
   * returns the monostate.
   *
   * The return type is a variant. You can pattern match on the variant to
   * handle different types. However, if you know what order the values were
   * written in (which you really should), you can use std::get<T>(...) to
   * just assert the next value should be of a certain type T and to extract
   * that value directly. This avoids the overhead of a pattern match on every
   * value. In addition, it is guaranteed to never corrupt the stream (unlike
   * the other read methods)
   */
  CustomMessage read();

  /**
   * Returns a single Serializable object.
   *
   * This method is only defined if {@link #nextType} has returned JsonType.
   * Otherwise, calling this method will potentially corrupt the stream.
   *
   * The method advances the read position. If called when no more data is
   * available, this method will return nullptr.
   *
   * @return a single Serializer object.
   */
  std::shared_ptr<Serializable> readSerializable();

  /**
   * Returns a vector of Serializable objects.
   *
   * This method is only defined if {@link #nextType} has returned JsonType.
   * Otherwise, calling this method will potentially corrupt the stream.
   *
   * The method advances the read position. If called when no more data is
   * available, this method will return nullptr.
   *
   * @return a vector of Serializer objects.
   */
  std::vector<std::shared_ptr<Serializable>> readSerializableVector();
};

}  // namespace cugl

#endif  // NETWORK_CUSTOM_NETWORK_SERIALIZER_H_
