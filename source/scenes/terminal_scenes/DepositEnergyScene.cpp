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
//  if (_done) {
//    auto info = cugl::JsonValue::allocObject();
//
//    auto terminal_room_id_info =
//        cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
//    info->appendChild(terminal_room_id_info);
//    terminal_room_id_info->setKey("terminal_room_id");
//
//    auto terminal_done_info = cugl::JsonValue::alloc(true);
//    info->appendChild(terminal_done_info);
//    terminal_done_info->setKey("terminal_done");
//
//    NetworkController::get()->sendOnlyToHost(NC_CLIENT_TERMINAL_DONE, info);
//
//    if (NetworkController::get()->isHost()) _voting_info->terminal_done = true;
//    return;
//  }

//  _done = (_voting_info->done.size() == _num_players_req);

//  auto found = std::find(_voting_info->done.begin(), _voting_info->done.end(),
//                         _player_controller->getMyPlayer()->getPlayerId());
//  if (found != _voting_info->done.end()) {
//    _corrupt_butt->setVisible(false);
//    _activate_butt->setVisible(false);
//  } else {
//    _corrupt_butt->setVisible(_is_betrayer);
//    _activate_butt->setVisible(true);
//  }

  _energy_bar->setProgress(
      static_cast<float>(_player_controller->getMyPlayer()->getHealth()) / 100);
  _corrupted_energy_bar->setProgress(
      static_cast<float>(_player_controller->getMyPlayer()->getLuminance()) /
      100);

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

  NetworkController::get()->sendOnlyToHost(NC_DEPOSIT_ENERGY, info);
  
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
