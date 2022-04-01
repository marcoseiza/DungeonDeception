#include "VoteForTeamScene.h"

#include "../../controllers/NetworkController.h"

bool VoteForTeamScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;

  _node = _assets->get<cugl::scene2::SceneNode>(
      "terminal-voting-scene_voting-background_vote-for-team");
  _node->setVisible(false);

  _initialized = true;
  return true;
}

void VoteForTeamScene::start(std::shared_ptr<VotingInfo> voting_info,
                             int terminal_room_id, int team_leader_id) {
  if (!_initialized || _active) return;
  _active = true;
  _voting_info = voting_info;
  _terminal_room_id = terminal_room_id;
  _team_leader_id = team_leader_id;

  int num_players = _voting_info->players.size();
  int i = 0;

  std::string leader_button_key =
      "terminal-voting-scene_voting-background_vote-for-team_leader-button-"
      "wrapper_leader-button";

  auto leader_button = _assets->get<cugl::scene2::SceneNode>(leader_button_key);

  if (_player_controller->getMyPlayer()->getPlayerId() == _team_leader_id) {
    auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
        _assets->get<cugl::scene2::SceneNode>(leader_button_key + "_up_label"));

    label->setText("player " + std::to_string(_team_leader_id));

    _assets
        ->get<cugl::scene2::SceneNode>(
            "terminal-voting-scene_voting-background_vote-for-team_title-1-not-"
            "leader")
        ->setVisible(false);
    _assets
        ->get<cugl::scene2::SceneNode>(
            "terminal-voting-scene_voting-background_vote-for-team_title-2-not-"
            "leader")
        ->setVisible(false);
  } else {
    _assets
        ->get<cugl::scene2::SceneNode>(
            "terminal-voting-scene_voting-background_vote-for-team_title-1")
        ->setVisible(false);
    _assets
        ->get<cugl::scene2::SceneNode>(
            "terminal-voting-scene_voting-background_vote-for-team_title-2")
        ->setVisible(false);
  }

  auto buttons = _assets->get<cugl::scene2::SceneNode>(
      "terminal-voting-scene_voting-background_vote-for-team_buttons");

  for (auto button : buttons->getChildren()) {
    std::string button_key = cugl::strtool::format(
        "terminal-voting-scene_voting-background_vote-for-team_buttons_"
        "button-wrapper-%d_button-%d",
        i, i);
    auto butt = std::dynamic_pointer_cast<cugl::scene2::Button>(
        _assets->get<cugl::scene2::SceneNode>(button_key));

    if (i < num_players) {
      int player_id = _voting_info->players[i];

      if (player_id != _team_leader_id) {
        _buttons[player_id] = butt;

        auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
            _assets->get<cugl::scene2::SceneNode>(button_key + "_up_label"));
        std::string name = "player " + std::to_string(player_id);
        label->setText(name, true);

        butt->setName(std::to_string(player_id));

        if (_player_controller->getMyPlayer()->getPlayerId() ==
            _team_leader_id) {
          butt->addListener([=](const std::string& name, bool down) {
            this->voteButtonListener(name, down);
          });

          butt->activate();
        } else {
          butt->deactivate();
        }

        butt->setVisible(true);

      } else {
        butt->setVisible(false);
        butt->deactivate();
      }

    } else {
      butt->setVisible(false);
      butt->deactivate();
    }
    i++;
  }

  _ready_button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>(
          "terminal-voting-scene_voting-background_vote-for-team_done-button-"
          "wrapper_done-button"));

  if (_player_controller->getMyPlayer()->getPlayerId() == _team_leader_id) {
    _ready_button->addListener([=](const std::string& name, bool down) {
      this->readyButtonListener(name, down);
    });
    _ready_button->activate();
  } else {
    _ready_button->setVisible(false);
  }

  _node->setVisible(true);
  _node->doLayout();
}

void VoteForTeamScene::update() {
  for (auto it : _buttons) {
    auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
        (it.second)->getChildByName("up")->getChildByName("num-votes"));
    label->setText("0");
  }

  for (int voted : _voting_info->votes[_team_leader_id]) {
    auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
        _buttons[voted]->getChildByName("up")->getChildByName("num-votes"));
    label->setText("1");
  }
}

void VoteForTeamScene::voteButtonListener(const std::string& name, bool down) {
  if (!down) return;

  {
    int voted_for = std::stoi(name);
    int player_id = _player_controller->getMyPlayer()->getPlayerId();

    _voting_info->votes[player_id].push_back(voted_for);
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
    auto voted_for_info = cugl::JsonValue::allocArray();
    std::vector<int>& voted_for =
        _voting_info->votes[_player_controller->getMyPlayer()->getPlayerId()];

    for (int player_voted_for_id : voted_for) {
      voted_for_info->appendChild(
          cugl::JsonValue::alloc(static_cast<long>(player_voted_for_id)));
    }

    info->appendChild(voted_for_info);
    voted_for_info->setKey("voted_for");
  }

  NetworkController::get()->sendOnlyToHost(8, info);
}

void VoteForTeamScene::readyButtonListener(const std::string& name, bool down) {
  if (!down || !_can_finish) return;

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
    auto should_add_info = cugl::JsonValue::alloc(true);
    info->appendChild(should_add_info);
    should_add_info->setKey("add");
  }

  NetworkController::get()->sendOnlyToHost(10, info);

  if (NetworkController::get()->isHost()) {
    int player_id = _player_controller->getMyPlayer()->getPlayerId();
    if (std::find(_voting_info->done.begin(), _voting_info->done.end(),
                  player_id) == _voting_info->done.end()) {
      _voting_info->done.push_back(player_id);
    }
  }
}
