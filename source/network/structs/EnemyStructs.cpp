#include "EnemyStructs.h"

namespace cugl {

#pragma mark -
#pragma mark EnemyInfo

const Uint32 EnemyInfo::Key = NC_HOST_ALL_ENEMY_INFO;

void EnemyInfo::serialize(cugl::NetworkSerializer* serializer) {
  serializer->writeUint32(enemy_id);

  serializer->writeDouble(pos.x);
  serializer->writeDouble(pos.y);

  serializer->writeBool(has_target);

  if (has_target) {
    serializer->writeDouble(target.x);
    serializer->writeDouble(target.y);
  }
}

void EnemyInfo::deserialize(cugl::NetworkDeserializer* deserializer) {
  enemy_id = std::get<Uint32>(deserializer->read());
  pos.x = std::get<double>(deserializer->read());
  pos.y = std::get<double>(deserializer->read());
  has_target = std::get<bool>(deserializer->read());
  if (has_target) {
    target.x = std::get<double>(deserializer->read());
    target.y = std::get<double>(deserializer->read());
  }
}

#pragma mark -
#pragma mark EnemyOtherInfo

const Uint32 EnemyOtherInfo::Key = NC_HOST_ALL_ENEMY_OTHER_INFO;

void EnemyOtherInfo::serialize(cugl::NetworkSerializer* serializer) {
  serializer->writeUint32(enemy_id);
  serializer->writeSint32(health);
}

void EnemyOtherInfo::deserialize(cugl::NetworkDeserializer* deserializer) {
  enemy_id = std::get<Uint32>(deserializer->read());
  health = std::get<Sint32>(deserializer->read());
}

#pragma mark -
#pragma mark EnemyHitInfo

const Uint32 EnemyHitInfo::Key = NC_CLIENT_ENEMY_HIT_INFO;

void EnemyHitInfo::serialize(cugl::NetworkSerializer* serializer) {
  serializer->writeUint32(enemy_id);
  serializer->writeUint32(player_id);
  serializer->writeUint32(amount);
  serializer->writeUint32(direction);
}

void EnemyHitInfo::deserialize(cugl::NetworkDeserializer* deserializer) {
  enemy_id = std::get<Uint32>(deserializer->read());
  player_id = std::get<Uint32>(deserializer->read());
  amount = std::get<Uint32>(deserializer->read());
  direction = std::get<Uint32>(deserializer->read());
}

}  // namespace cugl