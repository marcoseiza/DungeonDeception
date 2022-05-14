#include "PlayerStructs.h"

namespace cugl {

#pragma mark -
#pragma mark PlayerIdInfo

const Uint32 PlayerIdInfo::Key = NC_CLIENT_END_GAME;

void PlayerIdInfo::serialize(cugl::NetworkSerializer* serializer) {
  serializer->writeUint32(player_id);
}

void PlayerIdInfo::deserialize(cugl::NetworkDeserializer* deserializer) {
  player_id = std::get<Uint32>(deserializer->read());
}

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
#pragma mark PlayerOtherInfo

const Uint32 PlayerOtherInfo::Key = NC_HOST_ALL_PLAYER_OTHER_INFO;

void PlayerOtherInfo::serialize(cugl::NetworkSerializer* serializer) {
  serializer->writeUint32(player_id);
  serializer->writeSint32(energy);
  serializer->writeSint32(corruption);
}

void PlayerOtherInfo::deserialize(cugl::NetworkDeserializer* deserializer) {
  player_id = std::get<Uint32>(deserializer->read());
  energy = std::get<Sint32>(deserializer->read());
  corruption = std::get<Sint32>(deserializer->read());
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