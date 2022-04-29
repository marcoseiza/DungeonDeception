#include "DepositEnergyScene.h"

#include "../../network/NetworkController.h"

bool DepositEnergyScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;

  _node = _assets->get<cugl::scene2::SceneNode>(
      "terminal-deposit-scene_deposit-background_deposit-energy");
  _node->setVisible(false);
  
  _energy_bar = std::dynamic_pointer_cast<cugl::scene2::ProgressBar>(_node->getChildByName("energy-bar")->getChildByName("bar"));
  _corrupted_energy_bar = std::dynamic_pointer_cast<cugl::scene2::ProgressBar>(_node->getChildByName("corrupted-energy-bar")->getChildByName("bar"));
  
  _terminal_room_id = -1;
  _initialized = true;
  return true;
}

void DepositEnergyScene::start(int terminal_room_id) {
  if (!_initialized || _active) return;
  _active = true;
  _node->setVisible(true);
  
  _terminal_room_id = terminal_room_id;

  _deposit_butt = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _node->getChildByName("deposit-button-wrapper")
          ->getChildByName("button"));
  _deposit_butt->activate();
  _deposit_butt->setVisible(true);

  _deposit_butt->addListener([=](const std::string& name, bool down) {
    this->depositButtonListener("deposit", down);
  });

  _done_butt = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _node->getChildByName("done-button-wrapper")
          ->getChildByName("button"));

  _done_butt->addListener([=](const std::string& name, bool down) {
    this->doneButtonListener("done", down);
  });

  _done_butt->setVisible(true);
  _done_butt->activate();
}

void DepositEnergyScene::update() {
  if (_terminal_room_id != -1) {
    auto terminal_room = _level_controller->getLevelModel()->getRoom(_terminal_room_id);
    _energy_bar->setProgress(
        static_cast<float>(terminal_room->getEnergy() / 100.0f));
    _corrupted_energy_bar->setProgress(
        static_cast<float>(terminal_room->getCorruptedEnergy() / 100.0f));
  }
}

void DepositEnergyScene::depositButtonListener(const std::string& name, bool down) {
  if (!down) return;
  
  auto info = cugl::JsonValue::allocObject();
  
  auto terminal_room_id_info =
      cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
  info->appendChild(terminal_room_id_info);
  terminal_room_id_info->setKey("terminal_room_id");
  
  int player_id = _player_controller->getMyPlayer()->getPlayerId();
  auto player_id_info = cugl::JsonValue::alloc(static_cast<long>(player_id));
  info->appendChild(player_id_info);
  player_id_info->setKey("player_id");

  NetworkController::get()->sendOnlyToHostOrProcess(NC_DEPOSIT_ENERGY, info);
  
//  auto player = _player_controller->getMyPlayer();
//  int corrupted_luminance = player->getCorruptedLuminance();
//  int luminance = player->getLuminance();
//  int energy_to_deposit = _player_controller->getMyPlayer()->getLuminance();

}

void DepositEnergyScene::doneButtonListener(const std::string& name,
                                              bool down) {
  if (!down) return;

  // Uncomment to add networking
//  std::shared_ptr<cugl::JsonValue> info = cugl::JsonValue::allocObject();
//  int player_id = _player_controller->getMyPlayer()->getPlayerId();
//
//  {
//    std::shared_ptr<cugl::JsonValue> room_info = cugl::JsonValue::alloc(
//        static_cast<long>(_voting_info->terminal_room_id));
//    info->appendChild(room_info);
//    room_info->setKey("terminal_room_id");
//  }
//  {
//    std::shared_ptr<cugl::JsonValue> player_info =
//        cugl::JsonValue::alloc(static_cast<long>(player_id));
//    info->appendChild(player_info);
//    player_info->setKey("player_id");
//  }
//
//  NetworkController::get()->sendOnlyToHost(NC_CLIENT_PLAYER_REMOVED, info);
//
//  if (NetworkController::get()->isHost()) {
//    std::vector<int>& players = _voting_info->players;
//    players.erase(std::remove(players.begin(), players.end(), player_id),
//                  players.end());
//  }

  _exit = true;
}
