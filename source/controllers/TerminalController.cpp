#include "TerminalController.h"

#include "../network/NetworkController.h"
#include "../network/structs/TerminalStructs.h"

bool TerminalController::init(
    const std::shared_ptr<cugl::AssetManager> &assets) {
  if (_active) return false;

  _assets = assets;
  _scene = _assets->get<cugl::scene2::SceneNode>("terminal-deposit-scene");
  _scene->setVisible(false);

  _deposit_energy_scene = DepositEnergyScene::alloc(_assets);

  NetworkController::get()->addListener(
      [=](const Sint32 &code,
          const cugl::CustomNetworkDeserializer::CustomMessage &msg) {
        this->processNetworkData(code, msg);
      });

  return true;
}

void TerminalController::update(float timestep) {
  sendNetworkData();

  if (!_active) return;

  if (!_deposit_energy_scene->isActive()) {
    _deposit_energy_scene->start(_terminal_room_id);
  } else {
    _deposit_energy_scene->update();
  }

  if (_deposit_energy_scene->isDone() || _deposit_energy_scene->didExit()) {
    _deposit_energy_scene->dispose();
    done();
    _terminal_sensor->deactivate();
  }
}

void TerminalController::sendNetworkData() {
  //  if (NetworkController::get()->isHost()) {
  //
  //  }
}

void TerminalController::processNetworkData(
    const Sint32 &code,
    const cugl::CustomNetworkDeserializer::CustomMessage &msg) {
  switch (code) {
    case NC_DEPOSIT_ENERGY: {
      // Process the incoming informaiton
      auto info = std::dynamic_pointer_cast<cugl::TerminalDeposit>(
          std::get<std::shared_ptr<cugl::Serializable>>(msg));

      auto level_model = _level_controller->getLevelModel();
      auto player = _player_controller->getPlayer(info->player_id);
      auto terminal_room = level_model->getRoom(info->room_id);

      // Whether there the player has enough corrupted energy to activate the
      // terminal. If so, the corrupted energy will always take priority. Can be
      // changed in the future!

      int cor_energy = player->getCorruptedEnergy();
      int energy = player->getEnergy();
      // A betrayer's energy is corrupted energy.
      if (player->isBetrayer()) cor_energy = player->getEnergy();

      int energy_needed =
          terminal_room->getEnergyToActivate() - terminal_room->getEnergy();
      int cor_energy_needed = terminal_room->getCorruptedEnergyToActivate() -
                              terminal_room->getCorruptedEnergy();

      // First deposit corruption, if terminal is activated after then don't
      // deposit regular energy.
      int cor_energy_to_deposit = std::min(cor_energy_needed, cor_energy);
      terminal_room->setCorruptedEnergy(terminal_room->getCorruptedEnergy() +
                                        cor_energy_to_deposit);
      player->setCorruptedEnergy(cor_energy - cor_energy_to_deposit);

      if (!player->isBetrayer() && cor_energy_to_deposit < cor_energy_needed) {
        int energy_to_deposit = std::min(energy_needed, energy);
        terminal_room->setEnergy(terminal_room->getEnergy() +
                                 energy_to_deposit);
        player->setEnergy(energy - energy_to_deposit);
      }

      sendTerminalUpdate(player, terminal_room);
    } break;

    case NC_TERMINAL_ENERGY_UPDATE: {
      auto info = std::dynamic_pointer_cast<cugl::TerminalUpdate>(
          std::get<std::shared_ptr<cugl::Serializable>>(msg));

      auto terminal_room =
          _level_controller->getLevelModel()->getRoom(info->room_id);
      terminal_room->setEnergy(info->room_energy);
      terminal_room->setCorruptedEnergy(info->room_corrupted_energy);
    } break;
  }
}

void TerminalController::sendTerminalUpdate(
    const std::shared_ptr<Player> &player,
    const std::shared_ptr<RoomModel> &room) {
  auto info = cugl::TerminalUpdate::alloc();

  info->player_id = player->getPlayerId();
  info->player_energy = player->getEnergy();
  info->player_corrupted_energy = player->getCorruptedEnergy();

  info->room_id = room->getKey();
  info->room_energy = room->getEnergy();
  info->room_corrupted_energy = room->getCorruptedEnergy();

  NetworkController::get()->sendAndProcess(NC_TERMINAL_ENERGY_UPDATE, info);
}
