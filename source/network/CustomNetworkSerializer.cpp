
#include "CustomNetworkSerializer.h"

using namespace cugl;

#pragma mark -
#pragma mark NetworkSerializer
/**
 * Writes a single Serializable object.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. Pass the result of {@link #serialize} to the {@link
 * NetworkConnection} to send all values buffered up to this point.
 *
 * @param v The value to write
 */
void CustomNetworkSerializer::writeSerializable(
    const std::shared_ptr<Serializable>& v) {
  _data.push_back(SerializableType);
  writeUint32(v->key());
  v->serialize(this);
}

/**
 * Writes a vector of Serializable object.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. Pass the result of {@link #serialize} to the {@link
 * NetworkConnection} to send all values buffered up to this point.
 *
 * @param v The value to write
 */
void CustomNetworkSerializer::writeSerializableVector(
    std::vector<std::shared_ptr<Serializable>> v) {
  _data.push_back(ArrayType + SerializableType);
  writeUint64((Uint64)(v.size()));
  for (size_t i = 0; i < v.size(); i++) {
    writeSerializable(v[i]);
  }
}

#pragma mark -
#pragma mark CustomNetworkDeserializer

/**
 * Creates a new Network Deserializer on the stack. And inits the serializer
 * factory vlookup table.
 *
 * Network deserializers do not have any nontrivial state and so it is
 * unnecessary to use an init method. However, we do include a static {@link
 * #alloc} method for creating shared pointers.
 */
CustomNetworkDeserializer::CustomNetworkDeserializer() {
  _pos = 0;

  // Register EnemyInfo struct in the factory for deserialization.
  _factory[EnemyInfo::Key] = []() { return EnemyInfo::alloc(); };

  // Register EnemyOtherInfo struct in the factory for deserialization.
  _factory[EnemyOtherInfo::Key] = []() { return EnemyOtherInfo::alloc(); };

  // Register EnemyHitInfo struct in the factory for deserialization.
  _factory[EnemyHitInfo::Key] = []() { return EnemyHitInfo::alloc(); };

  // Register PlayerIdInfo struct in the factory for deserialization.
  _factory[PlayerIdInfo::Key] = []() { return PlayerIdInfo::alloc(); };

  // Register PlayerInfo struct in the factory for deserialization.
  _factory[PlayerInfo::Key] = []() { return PlayerInfo::alloc(); };

  // Register PlayerOtherInfo struct in the factory for deserialization.
  _factory[PlayerOtherInfo::Key] = []() { return PlayerOtherInfo::alloc(); };

  // Register BasicPlayerInfo struct in the factory for deserialization.
  _factory[BasicPlayerInfo::Key] = []() { return BasicPlayerInfo::alloc(); };

  // Register TerminalUpdate struct in the factory for deserialization.
  _factory[TerminalUpdate::Key] = []() { return TerminalUpdate::alloc(); };

  // Register TerminalDeposit struct in the factory for deserialization.
  _factory[TerminalDeposit::Key] = []() { return TerminalDeposit::alloc(); };
}

/**
 * Reads the next unreturned value or vector from the currently loaded byte
 * vector.
 *
 * A byte vector should be loaded with the {@link #receive} method. If nothing
 * is loaded, this will return the monostate. This method also advances the
 * read position. If the end of the vector is reached, this returns the
 * monostate.
 *
 * The return type is a variant. You can pattern match on the variant to
 * handle different types. However, if you know what order the values were
 * written in (which you really should), you can use std::get<T>(...) to just
 * assert the next value should be of a certain type T and to extract that
 * value directly. This avoids the overhead of a pattern match on every value.
 * In addition, it is guaranteed to never corrupt the stream (unlike the other
 * read methods)
 */
CustomNetworkDeserializer::CustomMessage CustomNetworkDeserializer::read() {
  if (_pos >= _data.size()) {
    return {};
  }

  switch (_data[_pos]) {
    case NoneType:
      _pos++;
      return {};

    case BooleanTrue:
      _pos++;
      return true;
    case BooleanFalse:
      _pos++;
      return false;
    case FloatType:
      return readFloat();
    case DoubleType:
      return readDouble();
    case UInt32Type:
      return readUint32();
    case UInt64Type:
      return readUint64();
    case SInt32Type:
      return readSint32();
    case SInt64Type:
      return readSint64();
    case StringType:
      return readString();
    case JsonType:
      return readJson();
    case SerializableType:
      return readSerializable();

    case ArrayType + BooleanTrue:
      return readBoolVector();
    case ArrayType + FloatType:
      return readFloatVector();
    case ArrayType + DoubleType:
      return readDoubleVector();
    case ArrayType + UInt32Type:
      return readUint32Vector();
    case ArrayType + UInt64Type:
      return readUint64Vector();
    case ArrayType + SInt32Type:
      return readSint32Vector();
    case ArrayType + SInt64Type:
      return readSint64Vector();
    case ArrayType + StringType:
      return readStringVector();
    case ArrayType + JsonType:
      return readJsonVector();
    case ArrayType + SerializableType:
      return readSerializableVector();

    default:
      throw std::domain_error(
          "Illegal state of array; did you pass in a valid message?");
  }
}

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
std::shared_ptr<Serializable> CustomNetworkDeserializer::readSerializable() {
  if (_pos >= _data.size()) {
    return 0;
  }
  _pos++;

  Uint32 key = std::get<Uint32>(read());
  std::shared_ptr<Serializable> res = _factory[key]();
  res->deserialize(this);
  return res;
}

/**
 * Returns a vector of Serializable objects.
 *
 * This method is only defined if {@link #nextType} has returned JsonType.
 * Otherwise, calling this method will potentially corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return nullptr.
 *
 * @return a vector Serializer objects.
 */
std::vector<std::shared_ptr<Serializable>>
CustomNetworkDeserializer::readSerializableVector() {
  std::vector<std::shared_ptr<Serializable>> vv;
  if (_pos >= _data.size()) {
    return vv;
  }
  _pos++;
  Uint64 size = std::get<Uint64>(read());
  for (size_t i = 0; i < size; i++) {
    vv.push_back(std::get<std::shared_ptr<Serializable>>(read()));
  }
  return vv;
}
