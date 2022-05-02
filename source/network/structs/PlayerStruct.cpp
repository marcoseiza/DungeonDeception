#include "PlayerStructs.h"

namespace cugl {

#pragma mark -
#pragma mark PlayerInfo

const Uint32 PlayerInfo::Key = NC_HOST_ALL_PLAYER_INFO;

void PlayerInfo::serialize(cugl::NetworkSerializer* serializer) {
  serializer->writeUint32(player_id);
  serializer->writeUint32(room_id);

  serializer->writeDouble(pos.x);
  serializer->writeDouble(pos.y);
}

void PlayerInfo::deserialize(cugl::NetworkDeserializer* deserializer) {
  player_id = std::get<Uint32>(deserializer->read());
  room_id = std::get<Uint32>(deserializer->read());

  pos.x = std::get<double>(deserializer->read());
  pos.y = std::get<double>(deserializer->read());
}

#pragma mark -
#pragma mark BasicPlayerInfo

const Uint32 BasicPlayerInfo::Key = NC_HOST_ALL_PLAYER_BASIC_INFO;

void BasicPlayerInfo::serialize(cugl::NetworkSerializer* serializer) {
  serializer->writeUint32(player_id);
  serializer->writeString(name);
  serializer->writeBool(betrayer);
}

void BasicPlayerInfo::deserialize(cugl::NetworkDeserializer* deserializer) {
  player_id = std::get<Uint32>(deserializer->read());
  name = std::get<std::string>(deserializer->read());
  betrayer = std::get<bool>(deserializer->read());
}

}  // namespace cugl