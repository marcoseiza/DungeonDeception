#include "VoteForLeaderScene.h"

#include "../../controllers/NetworkController.h"

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
    std::string button_key = cugl::strtool::format(
        "terminal-voting-scene_voting-background_vote-for-leader_buttons_"
        "button-wrapper-%d_button-%d",
        i, i);
    auto butt = std::dynamic_pointer_cast<cugl::scene2::Button>(
        _assets->get<cugl::scene2::SceneNode>(button_key));

    if (i < num_players) {
      int player_id = _voting_info->players[i];
      _buttons[player_id] = butt;

      auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
          _assets->get<cugl::scene2::SceneNode>(button_key + "_up_label"));
      std::string name = "player " + std::to_string(player_id);
      label->setText(name, true);

      butt->setName(std::to_string(player_id));

      butt->addListener([=](const std::string& name, bool down) {
        this->voteButtonListener(name, down);
      });

      butt->activate();

      butt->setVisible(true);

    } else {
      butt->setVisible(false);
      butt->deactivate();
    }
    i++;
  }

  _done_button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>(
          "terminal-voting-scene_voting-background_vote-for-leader_done-button-"
          "wrapper_done-button"));

  _done_button->addListener([=](const std::string& name, bool down) {
    this->doneButtonListener(name, down);
  });

  _node->setVisible(true);
  _node->doLayout();
}

void VoteForLeaderScene::update() {
  {  // Handle my player's input.
    std::vector<int>& votes =
        _voting_info->votes[_player_controller->getMyPlayer()->getPlayerId()];

    if (votes.size() == 1) {
      _done_button->activate();
      int player_id = votes[0];

      for (auto it : _buttons) {
        if (it.first == player_id) {
          it.second->deactivate();
        } else {
          it.second->activate();
        }
      }
    }
  }

  {  // Check the number of votes for all players.
    std::unordered_map<int, int> num_votes_per_person;

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
}

void VoteForLeaderScene::voteButtonListener(const std::string& name,
                                            bool down) {
  if (!down) return;

  int voted_for = std::stoi(name);

  std::vector<int>& votes =
      _voting_info->votes[_player_controller->getMyPlayer()->getPlayerId()];

  votes.clear();
  votes.push_back(voted_for);
}

void VoteForLeaderScene::doneButtonListener(const std::string& name,
                                            bool down) {
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

  NetworkController::get()->send(8, info);
}
