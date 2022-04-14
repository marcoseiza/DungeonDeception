#include "WaitForPlayersScene.h"

#include "../../controllers/NetworkController.h"

bool WaitForPlayersScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;

  _node = _assets->get<cugl::scene2::SceneNode>(
      "terminal-voting-scene_voting-background_wait-for-player");
  _node->setVisible(false);

  auto leave_butt = _node->getChildByName<cugl::scene2::Button>("leave");

  leave_butt->addListener([=](const std::string& name, bool down) {
    this->leaveButtonListener(name, down);
  });

  leave_butt->activate();

  _initialized = true;
  return true;
}

void WaitForPlayersScene::start(std::shared_ptr<VotingInfo> voting_info,
                                int num_players_req) {
  if (!_initialized || _active) return;
  _active = true;
  _node->setVisible(true);

  _voting_info = voting_info;
  _num_players_req = num_players_req;
  auto num_required_label = std::dynamic_pointer_cast<cugl::scene2::Label>(
      _node->getChildByName("num-required"));
  num_required_label->setText(
      "Number Of People Required: " + std::to_string(num_players_req), true);

  _node->doLayout();
}

void WaitForPlayersScene::update() {
  _curr_num_players = _voting_info->players.size();

  if (_voting_info->buffer_timer > WAIT_TIME_AFTER_REQUIRED_ACCOMPLISHED) {
    _done = true;  // DONE!
    return;
  }

  auto current_num_label = std::dynamic_pointer_cast<cugl::scene2::Label>(
      _node->getChildByName("current-num"));

  current_num_label->setText(
      "Current Number Of Players: " + std::to_string(_curr_num_players), true);
}

void WaitForPlayersScene::leaveButtonListener(const std::string& name,
                                              bool down) {
  if (!down) return;

  std::shared_ptr<cugl::JsonValue> info = cugl::JsonValue::allocObject();
  int player_id = _player_controller->getMyPlayer()->getPlayerId();

  {
    std::shared_ptr<cugl::JsonValue> room_info = cugl::JsonValue::alloc(
        static_cast<long>(_voting_info->terminal_room_id));
    info->appendChild(room_info);
    room_info->setKey("terminal_room_id");
  }
  {
    std::shared_ptr<cugl::JsonValue> player_info =
        cugl::JsonValue::alloc(static_cast<long>(player_id));
    info->appendChild(player_info);
    player_info->setKey("player_id");
  }

  NetworkController::get()->sendOnlyToHost(NC_CLIENT_PLAYER_REMOVED, info);

  if (NetworkController::get()->isHost()) {
    std::vector<int>& players = _voting_info->players;
    players.erase(std::remove(players.begin(), players.end(), player_id),
                  players.end());
  }

  _exit = true;
}
