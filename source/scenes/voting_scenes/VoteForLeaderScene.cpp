#include "VoteForLeaderScene.h"

#include "../../network/NetworkController.h"

bool VoteForLeaderScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;

  _node = _assets->get<cugl::scene2::SceneNode>(
      "terminal-voting-scene_voting-background_vote-for-leader");
  _node->setVisible(false);

  _initialized = true;
  return true;
}

void VoteForLeaderScene::start(std::shared_ptr<VotingInfo> voting_info,
                               int terminal_room_id) {
  if (!_initialized || _active) return;
  _active = true;
  _voting_info = voting_info;
  _terminal_room_id = terminal_room_id;

  int num_players = _voting_info->players.size();
  int i = 0;

  auto buttons = _assets->get<cugl::scene2::SceneNode>(
      "terminal-voting-scene_voting-background_vote-for-leader_buttons");

  for (auto button : buttons->getChildren()) {
    auto butt = std::dynamic_pointer_cast<cugl::scene2::Button>(
        button->getChildByName("button"));

    if (i < num_players) {
      int player_id = _voting_info->players[i];
      _buttons[player_id] = butt;

      auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
          butt->getChildByName("up")->getChildByName("label"));

      std::stringstream ss;
      ss << _player_controller->getPlayer(player_id)->getDisplayName();
      if (_player_controller->getMyPlayer()->getPlayerId() == player_id) {
        ss << " (you)";
      }
      label->setText(ss.str().c_str(), true);

      butt->addListener([=](const std::string& name, bool down) {
        this->voteButtonListener(std::to_string(player_id), down);
      });

      butt->activate();

      butt->setVisible(true);

      butt->getChildByName("up")->getChildByName("patch")->setColor(
          cugl::Color4::WHITE);

    } else {
      butt->setVisible(false);
      butt->deactivate();
    }
    i++;
  }

  _ready_button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>(
          "terminal-voting-scene_voting-background_vote-for-leader_done-button-"
          "wrapper_done-button"));

  _ready_button->addListener([=](const std::string& name, bool down) {
    this->readyButtonListener(name, down);
  });
  _ready_button->activate();
  _ready_button->setVisible(false);

  _node->setVisible(true);
  _node->doLayout();
}

void VoteForLeaderScene::update() {
  if (_done) return;

  std::unordered_map<int, int> num_votes_per_person;

  {  // Check the number of votes for all players.
    for (int player_id : _voting_info->players) {
      for (int vote_player_id : _voting_info->votes[player_id]) {
        num_votes_per_person[vote_player_id] = 0;
      }
    }

    for (int player_id : _voting_info->players) {
      std::vector<int>& votes = _voting_info->votes[player_id];
      for (int vote_player_id : votes) {
        num_votes_per_person[vote_player_id] += 1;
      }
    }

    for (auto it : _buttons) {
      auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
          it.second->getChildByName("up")->getChildByName("num-votes"));
      label->setText(std::to_string(num_votes_per_person[it.first]));
    }
  }

  {
    bool all_votes_in = true;
    for (int player_id : _voting_info->players) {
      all_votes_in &= (_voting_info->votes[player_id].size() == 1);
    }

    if (all_votes_in) {
      int winner = -1;
      int max_votes = 0;
      int number_of_max = 1;
      for (auto it : num_votes_per_person) {
        if (it.second > max_votes) {
          max_votes = it.second;
          winner = it.first;
          number_of_max = 1;
        } else if (it.second == max_votes) {
          number_of_max++;
        }
      }
      if (number_of_max == 1) {
        _can_finish = true;
        _ready_button->activate();
        _ready_button->setVisible(true);
        _winner = winner;
      } else {
        _can_finish = false;
        _has_clicked_ready = false;
        _ready_button->deactivate();
        _ready_button->setVisible(false);
        for (auto it : _buttons) it.second->activate();
        _voting_info->done.clear();
        removeAllPlayersFromDoneList();
      }
    }
  }

  // If everyone has pressed ready and there's a clear winner.
  _done = (_voting_info->done.size() == _voting_info->players.size());

  if (_has_clicked_ready) {
    _ready_button->deactivate();
    for (auto it : _buttons) it.second->deactivate();
  } else {
    // Process my player's input.
    std::vector<int>& votes =
        _voting_info->votes[_player_controller->getMyPlayer()->getPlayerId()];

    if (votes.size() == 1) {
      int player_id = votes[0];

      for (auto it : _buttons) {
        if (it.first == player_id) {
          it.second->getChildByName("up")->getChildByName("patch")->setColor(
              cugl::Color4("#4C7953"));
        } else {
          it.second->getChildByName("up")->getChildByName("patch")->setColor(
              cugl::Color4::WHITE);
        }
      }
    }
  }
}

void VoteForLeaderScene::removeAllPlayersFromDoneList() {
  if (!NetworkController::get()->isHost()) return;

  for (int player_id : _voting_info->players) {
    auto info = cugl::JsonValue::allocObject();

    {
      auto terminal_room_id_info =
          cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
      info->appendChild(terminal_room_id_info);
      terminal_room_id_info->setKey("terminal_room_id");
    }

    {
      auto player_id_info =
          cugl::JsonValue::alloc(static_cast<long>(player_id));
      info->appendChild(player_id_info);
      player_id_info->setKey("player_id");
    }

    {
      auto should_add_info = cugl::JsonValue::alloc(false);
      info->appendChild(should_add_info);
      should_add_info->setKey("add");
    }

    NetworkController::get()->sendOnlyToHost(NC_CLIENT_DONE_WITH_VOTE, info);
  }
}

void VoteForLeaderScene::voteButtonListener(const std::string& name,
                                            bool down) {
  if (!down || _has_clicked_ready) return;

  {
    int voted_for = std::stoi(name);
    int player_id = _player_controller->getMyPlayer()->getPlayerId();

    _voting_info->votes[player_id].clear();
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

  NetworkController::get()->sendOnlyToHost(NC_CLIENT_VOTING_INFO, info);
}

void VoteForLeaderScene::readyButtonListener(const std::string& name,
                                             bool down) {
  if (!down || !_can_finish) return;

  _has_clicked_ready = true;

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
