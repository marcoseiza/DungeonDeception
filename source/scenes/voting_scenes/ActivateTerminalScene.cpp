#include "ActivateTerminalScene.h"

#include "../../network/NetworkController.h"

bool ActivateTerminalScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;

  _node = _assets->get<cugl::scene2::SceneNode>(
      "terminal-voting-scene_voting-background_activate-terminal");
  _node->setVisible(false);

  _initialized = true;
  return true;
}

void ActivateTerminalScene::start(std::shared_ptr<VotingInfo> voting_info,
                                  int terminal_room_id, int num_players_req) {
  if (!_initialized || _active) return;
  _active = true;
  _voting_info = voting_info;
  _terminal_room_id = terminal_room_id;
  _num_players_req = num_players_req;
  _node->setVisible(true);
  _is_betrayer = _player_controller->getMyPlayer()->isBetrayer();

  _activate_butt = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _node->getChildByName("activate-button-wrapper")
          ->getChildByName("button"));
  _activate_butt->activate();
  _activate_butt->setVisible(true);

  _activate_butt->addListener([=](const std::string& name, bool down) {
    this->buttonListener("activate", down);
  });

  _corrupt_butt = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _node->getChildByName("corrupt-button-wrapper")
          ->getChildByName("button"));

  _corrupt_butt->addListener([=](const std::string& name, bool down) {
    this->buttonListener("corrupt", down);
  });

  _corrupt_butt->setVisible(_is_betrayer);
  if (_is_betrayer) {
    _corrupt_butt->activate();
  } else {
    _corrupt_butt->deactivate();
  }
}

void ActivateTerminalScene::update() {
  if (_done) {
    auto info = cugl::JsonValue::allocObject();

    {
      auto terminal_room_id_info =
          cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
      info->appendChild(terminal_room_id_info);
      terminal_room_id_info->setKey("terminal_room_id");
    }

    {
      auto terminal_done_info = cugl::JsonValue::alloc(true);
      info->appendChild(terminal_done_info);
      terminal_done_info->setKey("terminal_done");
    }

    NetworkController::get()->sendOnlyToHost(NC_CLIENT_TERMINAL_DONE, info);

    if (NetworkController::get()->isHost()) _voting_info->terminal_done = true;
    return;
  }

  _done = (_voting_info->done.size() == _num_players_req);

  auto found = std::find(_voting_info->done.begin(), _voting_info->done.end(),
                         _player_controller->getMyPlayer()->getPlayerId());
  if (found != _voting_info->done.end()) {
    _corrupt_butt->setVisible(false);
    _activate_butt->setVisible(false);
  } else {
    _corrupt_butt->setVisible(_is_betrayer);
    _activate_butt->setVisible(true);
  }
}

void ActivateTerminalScene::buttonListener(const std::string& name, bool down) {
  if (!down) return;

  if (name == "activate") {
    _did_activate = true;
  } else if (name == "corrupt") {
    _did_activate = false;
  }

  auto info = cugl::JsonValue::allocObject();

  {
    auto terminal_room_id_info =
        cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
    info->appendChild(terminal_room_id_info);
    terminal_room_id_info->setKey("terminal_room_id");
  }

  {
    int player_id = _player_controller->getMyPlayer()->getPlayerId();
    auto player_id_info = cugl::JsonValue::alloc(static_cast<long>(player_id));
    info->appendChild(player_id_info);
    player_id_info->setKey("player_id");
  }

  {
    auto did_activate_info = cugl::JsonValue::alloc(_did_activate);
    info->appendChild(did_activate_info);
    did_activate_info->setKey("did_activate");
  }

  NetworkController::get()->sendOnlyToHost(NC_CLIENT_DONE_ACTIVATE_TERMINAL,
                                           info);

  if (NetworkController::get()->isHost()) {
    int player_id = _player_controller->getMyPlayer()->getPlayerId();
    if (std::find(_voting_info->done.begin(), _voting_info->done.end(),
                  player_id) == _voting_info->done.end()) {
      _voting_info->was_activated &= _did_activate;
      _voting_info->done.push_back(player_id);
    }
  }
}