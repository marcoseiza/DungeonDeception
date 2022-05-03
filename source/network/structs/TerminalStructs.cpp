#include "TerminalStructs.h"

namespace cugl {

#pragma mark -
#pragma mark TerminalDeposit

const Uint32 TerminalDeposit::Key = NC_DEPOSIT_ENERGY;

void TerminalDeposit::serialize(cugl::NetworkSerializer* serializer) {
  serializer->writeUint32(player_id);
  serializer->writeUint32(room_id);
}

void TerminalDeposit::deserialize(cugl::NetworkDeserializer* deserializer) {
  player_id = std::get<Uint32>(deserializer->read());
  room_id = std::get<Uint32>(deserializer->read());
}

#pragma mark -
#pragma mark TerminalUpdate

const Uint32 TerminalUpdate::Key = NC_TERMINAL_ENERGY_UPDATE;

void TerminalUpdate::serialize(cugl::NetworkSerializer* serializer) {
  serializer->writeUint32(player_id);
  serializer->writeUint32(player_energy);
  serializer->writeUint32(player_corrupted_energy);
  serializer->writeUint32(room_id);
  serializer->writeUint32(room_energy);
  serializer->writeUint32(room_corrupted_energy);
}

void TerminalUpdate::deserialize(cugl::NetworkDeserializer* deserializer) {
  player_id = std::get<Uint32>(deserializer->read());
  player_energy = std::get<Uint32>(deserializer->read());
  player_corrupted_energy = std::get<Uint32>(deserializer->read());
  room_id = std::get<Uint32>(deserializer->read());
  room_energy = std::get<Uint32>(deserializer->read());
  room_corrupted_energy = std::get<Uint32>(deserializer->read());
}

}  // namespace cugl