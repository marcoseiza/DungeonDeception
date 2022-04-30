#include "TerminalController.h"

#include "../network/NetworkController.h"

bool TerminalController::init(
    const std::shared_ptr<cugl::AssetManager> &assets) {
  if (_active) return false;

  _assets = assets;
  _scene = _assets->get<cugl::scene2::SceneNode>("terminal-deposit-scene");
  _scene->setVisible(false);

  _deposit_energy_scene = DepositEnergyScene::alloc(_assets);

  NetworkController::get()->addListener(
      [=](const Sint32 &code, const cugl::NetworkDeserializer::Message &msg) {
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
    const Sint32 &code, const cugl::NetworkDeserializer::Message &msg) {
  switch (code) {
    case NC_DEPOSIT_ENERGY: {
      // Process the incoming informaiton
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);
      int player_id = info->getInt("player_id");
      int terminal_room_id = info->getInt("terminal_room_id");

      auto level_model = _level_controller->getLevelModel();
      auto player = _player_controller->getPlayer(player_id);
      int corrupted_energy = player->getCorruptedLuminance();
      int energy = player->getLuminance() - corrupted_energy;

      auto terminal_room = level_model->getRoom(terminal_room_id);

      // Whether there the player has enough corrupted energy to activate the
      // terminal. If so, the corrupted energy will always take priority. Can be
      // changed in the future!
      bool will_reach_max_corruption =
          terminal_room->getCorruptedEnergyToActivate() -
              terminal_room->getCorruptedEnergy() <=
          corrupted_energy;

      bool will_reach_max_energy =
          terminal_room->getEnergyToActivate() - terminal_room->getEnergy() <=
          energy;

      if (player->isBetrayer()) {
        if (will_reach_max_energy) {
          // Use up all corrupted energy and no regular energy.
          terminal_room->setCorruptedEnergy(
              terminal_room->getCorruptedEnergyToActivate());
          player->setLuminance(corrupted_energy -
                               (terminal_room->getCorruptedEnergyToActivate() -
                                terminal_room->getCorruptedEnergy()));
        } else {
          terminal_room->setCorruptedEnergy(
              terminal_room->getCorruptedEnergy() + energy);
        }
      } else {
        if (will_reach_max_corruption) {
          // Use up all corrupted energy and no regular energy.
          terminal_room->setCorruptedEnergy(
              terminal_room->getCorruptedEnergyToActivate());
          player->setCorruptedLuminance(
              corrupted_energy -
              (terminal_room->getCorruptedEnergyToActivate() -
               terminal_room->getCorruptedEnergy()));
        } else if (will_reach_max_energy) {
          terminal_room->setCorruptedEnergy(
              terminal_room->getCorruptedEnergy() + corrupted_energy);
          player->setCorruptedLuminance(0);
          terminal_room->setEnergy(terminal_room->getEnergyToActivate());
          player->setLuminance(energy - (terminal_room->getEnergyToActivate() -
                                         terminal_room->getEnergy()));
        } else {
          terminal_room->setCorruptedEnergy(
              terminal_room->getCorruptedEnergy() + corrupted_energy);
          terminal_room->setEnergy(terminal_room->getEnergy() + energy);
        }
      }

      sendDepositEnergySuccess();
      sendTerminalEnergyUpdate();

    } break;

    case NC_TERMINAL_ENERGY_UPDATE: {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);
      int terminal_room_id = info->getInt("terminal_room_id");
      int energy = info->getInt("energy");
      int corrupted_energy = info->getInt("corrupt_energy");
      auto terminal_room =
          _level_controller->getLevelModel()->getRoom(terminal_room_id);
      terminal_room->setEnergy(energy);
      terminal_room->setCorruptedEnergy(corrupted_energy);
    } break;
  }
}

void TerminalController::sendDepositEnergySuccess() {
  // Send the success message to all clients
  auto success_info = cugl::JsonValue::allocObject();

  auto player_id_info = cugl::JsonValue::alloc((long)(player_id));
  success_info->appendChild(player_id_info);
  player_id_info->setKey("player_id");

  auto updated_energy_info =
      cugl::JsonValue::alloc((long)(player->getLuminance()));
  success_info->appendChild(updated_energy_info);
  updated_energy_info->setKey("energy");

  auto updated_corrupt_energy_info =
      cugl::JsonValue::alloc((long)(player->getCorruptedLuminance()));
  success_info->appendChild(updated_corrupt_energy_info);
  updated_corrupt_energy_info->setKey("corrupt_energy");

  NetworkController::get()->sendAndProcess(NC_DEPOSIT_ENERGY_SUCCESS, info);
}

void TerminalController::sendTerminalEnergyUpdate() {
  auto terminal_update_info = cugl::JsonValue::allocObject();

  auto terminal_id_info = cugl::JsonValue::alloc((long)(terminal_room_id));
  terminal_update_info->appendChild(terminal_id_info);
  terminal_id_info->setKey("terminal_room_id");

  auto updated_terminal_energy =
      cugl::JsonValue::alloc((long)(terminal_room->getEnergy()));
  terminal_update_info->appendChild(updated_terminal_energy);
  updated_terminal_energy->setKey("energy");

  auto updated_terminal_corrupted_energy =
      cugl::JsonValue::alloc((long)(terminal_room->getCorruptedEnergy()));
  terminal_update_info->appendChild(updated_terminal_corrupted_energy);
  updated_terminal_corrupted_energy->setKey("corrupt_energy");

  NetworkController::get()->sendAndProcess(NC_TERMINAL_ENERGY_UPDATE,
                                           terminal_update_info);
}
