#include "TerminalController.h"

#include "../network/NetworkController.h"
#include "../network/structs/TerminalStructs.h"

bool TerminalController::init(
    const std::shared_ptr<cugl::AssetManager> &assets) {
  if (_active) return false;

  _assets = assets;
  _scene = _assets->get<cugl::scene2::SceneNode>("terminal-deposit-scene");
  _scene->setVisible(false);

  _num_terminals_activated = 0;
  _num_terminals_corrupted = 0;

  _deposit_energy_scene = DepositEnergyScene::alloc(_assets);

  NetworkController::get()->addListener(
      [=](const Sint32 &code,
          const cugl::CustomNetworkDeserializer::CustomMessage &msg) {
        this->processNetworkData(code, msg);
      });

  return true;
}

void TerminalController::update(float timestep) {
  if (!_active) return;

  if (!_deposit_energy_scene->isActive()) {
    _deposit_energy_scene->start(_terminal_room_id);
  } else {
    _deposit_energy_scene->update();
  }

  if (_deposit_energy_scene->isDone() || _deposit_energy_scene->didExit()) {
    _deposit_energy_scene->dispose();
    done();
  }
}

void TerminalController::processNetworkData(
    const Sint32 &code,
    const cugl::CustomNetworkDeserializer::CustomMessage &msg) {
  switch (code) {
    case NC_DEPOSIT_ENERGY: {  // ONLY HOST
      // Process the incoming informaiton
      auto info = std::dynamic_pointer_cast<cugl::TerminalDeposit>(
          std::get<std::shared_ptr<cugl::Serializable>>(msg));

      auto level_model = _level_controller->getLevelModel();
      auto terminal = level_model->getRoom(info->room_id);

      int energy_needed =
          terminal->getEnergyToActivate() - terminal->getEnergy();
      int cor_energy_needed = terminal->getCorruptedEnergyToActivate() -
                              terminal->getCorruptedEnergy();
      // Don't let players deposit if the terminal is already activated.
      if (energy_needed <= 0 || cor_energy_needed <= 0) break;

      auto player = _player_controller->getPlayer(info->player_id);
      int cor_energy = player->getCorruptedEnergy();
      int energy = player->getEnergy() - cor_energy;
      // A betrayer's energy is corrupted energy.
      if (player->isBetrayer()) cor_energy = player->getEnergy();

      // Whether there the player has enough corrupted energy to activate the
      // terminal. If so, the corrupted energy will always take priority. Can be
      // changed in the future!

      // First deposit corruption, if terminal is activated after then don't
      // deposit regular energy.
      int cor_energy_to_deposit = std::min(cor_energy_needed, cor_energy);
      terminal->setCorruptedEnergy(terminal->getCorruptedEnergy() +
                                   cor_energy_to_deposit);

      if (player->isBetrayer()) {
        player->setEnergy(cor_energy - cor_energy_to_deposit);

      } else {
        player->setCorruptedEnergy(cor_energy - cor_energy_to_deposit);

        if (cor_energy_to_deposit < cor_energy_needed) {
          int energy_to_deposit = std::min(energy_needed, energy);
          terminal->setEnergy(terminal->getEnergy() + energy_to_deposit);
          player->setEnergy(energy - energy_to_deposit);
        }
      }

      if (terminal->getEnergy() >= terminal->getEnergyToActivate()) {
        _num_terminals_activated++;
      } else if (terminal->getCorruptedEnergy() >=
                 terminal->getCorruptedEnergyToActivate()) {
        _num_terminals_corrupted++;
      }

      sendTerminalUpdate(player, terminal);
    } break;

    case NC_TERMINAL_ENERGY_UPDATE: {
      auto info = std::dynamic_pointer_cast<cugl::TerminalUpdate>(
          std::get<std::shared_ptr<cugl::Serializable>>(msg));

      auto terminal_room =
          _level_controller->getLevelModel()->getRoom(info->room_id);
      terminal_room->setEnergy(info->room_energy);
      terminal_room->setCorruptedEnergy(info->room_corrupted_energy);

      _num_terminals_activated = info->num_terminals_activated;
      _num_terminals_corrupted = info->num_terminals_corrupted;
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

  info->num_terminals_activated = _num_terminals_activated;
  info->num_terminals_corrupted = _num_terminals_corrupted;

  NetworkController::get()->sendAndProcess(NC_TERMINAL_ENERGY_UPDATE, info);
}

void TerminalController::done() {
  _active = false;
  _scene->setVisible(false);
  InputController::get()->resume();
}