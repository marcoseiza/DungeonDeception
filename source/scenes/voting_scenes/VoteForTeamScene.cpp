#include "VoteForTeamScene.h"

#include "../../network/NetworkController.h"

bool VoteForTeamScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;

  _node = _assets->get<cugl::scene2::SceneNode>(
      "terminal-voting-scene_voting-background_vote-for-team");
  _node->setVisible(false);

  _initialized = true;
  return true;
}

void VoteForTeamScene::start(std::shared_ptr<VotingInfo> voting_info,
                             int terminal_room_id, int team_leader_id,
                             int num_players_req) {
  if (!_initialized || _active) return;
  _active = true;
  _voting_info = voting_info;
  _terminal_room_id = terminal_room_id;
  _team_leader_id = team_leader_id;
  _num_players_req = num_players_req;

  {
    std::string leader_button_key =
        "terminal-voting-scene_voting-background_vote-for-team_leader-button-"
        "wrapper_leader-button";

    auto leader_button =
        _assets->get<cugl::scene2::SceneNode>(leader_button_key);

    auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
        _assets->get<cugl::scene2::SceneNode>(leader_button_key + "_up_label"));

    std::stringstream ss;
    ss << _player_controller->getPlayer(team_leader_id)->getDisplayName();
    if (_player_controller->getMyPlayer()->getPlayerId() == _team_leader_id) {
      ss << " (you)";
    }
    label->setText(ss.str().c_str(), true);
  }

  if (_player_controller->getMyPlayer()->getPlayerId() == _team_leader_id) {
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
    _assets
        ->get<cugl::scene2::SceneNode>(
            "terminal-voting-scene_voting-background_vote-for-team_title-1")
        ->setVisible(true);
    _assets
        ->get<cugl::scene2::SceneNode>(
            "terminal-voting-scene_voting-background_vote-for-team_title-2")
        ->setVisible(true);

    std::dynamic_pointer_cast<cugl::scene2::Label>(
        _assets->get<cugl::scene2::SceneNode>(
            "terminal-voting-scene_voting-background_vote-for-team_title-2"))
        ->setText("TEAM OF " + std::to_string(_num_players_req - 1), true);
  } else {
    _assets
        ->get<cugl::scene2::SceneNode>(
            "terminal-voting-scene_voting-background_vote-for-team_title-1")
        ->setVisible(false);
    _assets
        ->get<cugl::scene2::SceneNode>(
            "terminal-voting-scene_voting-background_vote-for-team_title-2")
        ->setVisible(false);

    _assets
        ->get<cugl::scene2::SceneNode>(
            "terminal-voting-scene_voting-background_vote-for-team_title-1-not-"
            "leader")
        ->setVisible(true);
    _assets
        ->get<cugl::scene2::SceneNode>(
            "terminal-voting-scene_voting-background_vote-for-team_title-2-not-"
            "leader")
        ->setVisible(true);
  }

  auto buttons = _assets->get<cugl::scene2::SceneNode>(
      "terminal-voting-scene_voting-background_vote-for-team_buttons");

  for (auto button : buttons->getChildren()) {
    auto butt = std::dynamic_pointer_cast<cugl::scene2::Button>(
        button->getChildByName("button"));

    butt->getChildByName("up")->getChildByName("patch")->setColor(
        cugl::Color4::WHITE);

    butt->setVisible(false);
    butt->deactivate();
  }

  int num_players = _voting_info->players.size();
  int i = 0;

  for (int player_id : _voting_info->players) {
    if (player_id != _team_leader_id) {
      auto butt = std::dynamic_pointer_cast<cugl::scene2::Button>(
          buttons->getChild(i)->getChildByName("button"));

      _buttons[player_id] = butt;

      auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
          butt->getChildByName("up")->getChildByName("label"));

      std::stringstream ss;
      ss << _player_controller->getPlayer(player_id)->getDisplayName();
      if (_player_controller->getMyPlayer()->getPlayerId() == player_id) {
        ss << " (you)";
      }
      label->setText(ss.str().c_str(), true);

      butt->getChildByName("up")
          ->getChildByName("num-votes")
          ->setVisible(false);

      if (_player_controller->getMyPlayer()->getPlayerId() == _team_leader_id) {
        butt->addListener([=](const std::string& name, bool down) {
          this->voteButtonListener(std::to_string(player_id), down);
        });

        butt->activate();
      } else {
        butt->deactivate();
      }

      butt->setVisible(true);

      i++;
    }
  }

  _ready_button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>(
          "terminal-voting-scene_voting-background_vote-for-team_done-button-"
          "wrapper_done-button"));

  if (_player_controller->getMyPlayer()->getPlayerId() == _team_leader_id) {
    _ready_button->addListener([=](const std::string& name, bool down) {
      this->readyButtonListener(name, down);
    });
  }

  _ready_button->setVisible(false);
  _ready_button->deactivate();

  _node->setVisible(true);
  _node->doLayout();
}

void VoteForTeamScene::update() {
  for (auto it : _buttons) {
    it.second->getChildByName("up")->getChildByName("patch")->setColor(
        cugl::Color4::WHITE);
  }

  for (int voted : _voting_info->votes[_team_leader_id]) {
    if (_buttons.find(voted) != _buttons.end()) {
      _buttons[voted]->getChildByName("up")->getChildByName("patch")->setColor(
          cugl::Color4("#4C7953"));
    }
  }

  _done = (_voting_info->done.size() == 1);

  if (_voting_info->votes[_team_leader_id].size() == _num_players_req - 1) {
    _can_finish = true;

    if (_player_controller->getMyPlayer()->getPlayerId() == _team_leader_id) {
      _ready_button->activate();
      _ready_button->setVisible(true);
    }
  } else {
    _can_finish = false;

    if (_player_controller->getMyPlayer()->getPlayerId() == _team_leader_id) {
      _ready_button->deactivate();
      _ready_button->setVisible(false);
    }
  }
}

void VoteForTeamScene::voteButtonListener(const std::string& name, bool down) {
  if (!down) return;

  int voted_for = std::stoi(name);
  std::vector<int>& votes = _voting_info->votes[_team_leader_id];

  {
    auto found = std::find(votes.begin(), votes.end(), voted_for);
    if (found != votes.end()) {
      votes.erase(found);
    } else {
      votes.push_back(voted_for);
    }
  }

  auto info = cugl::JsonValue::allocObject();

  {
    auto terminal_room_id_info =
        cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
    info->appendChild(terminal_room_id_info);
    terminal_room_id_info->setKey("terminal_room_id");
  }

  {
    auto player_id_info =
        cugl::JsonValue::alloc(static_cast<long>(_team_leader_id));
    info->appendChild(player_id_info);
    player_id_info->setKey("player_id");
  }

  {
    auto voted_for_info = cugl::JsonValue::allocArray();

    for (int player_voted_for_id : votes) {
      voted_for_info->appendChild(
          cugl::JsonValue::alloc(static_cast<long>(player_voted_for_id)));
    }

    info->appendChild(voted_for_info);
    voted_for_info->setKey("voted_for");
  }

  NetworkController::get()->sendOnlyToHost(NC_CLIENT_VOTING_INFO, info);
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

  NetworkController::get()->sendOnlyToHost(NC_CLIENT_DONE_WITH_VOTE, info);

  if (NetworkController::get()->isHost()) {
    int player_id = _player_controller->getMyPlayer()->getPlayerId();
    if (std::find(_voting_info->done.begin(), _voting_info->done.end(),
                  player_id) == _voting_info->done.end()) {
      _voting_info->done.push_back(player_id);
    }
  }
}
