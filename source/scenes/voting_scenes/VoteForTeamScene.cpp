#include "VoteForLeaderScene.h"

bool VoteForLeaderScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;

  _node = _assets->get<cugl::scene2::SceneNode>(
      "terminal-voting-scene_voting-background_vote-for-leader");
  // _node->setVisible(false);

  _initialized = true;
  return true;
}

void VoteForLeaderScene::start(std::shared_ptr<VotingInfo> voting_info) {
  if (!_initialized || _active) return;
  _active = true;

  voting_info = std::make_shared<VotingInfo>();
  voting_info->players = std::vector<int>{1, 2, 3};

  _voting_info = voting_info;

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

      auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
          _assets->get<cugl::scene2::SceneNode>(button_key + "_up_label"));
      std::string name = "player " + std::to_string(player_id);
      label->setText(name, true);

      butt->setName(std::to_string(player_id));

      butt->addListener([=](const std::string& name, bool down) {
        this->buttonListener(butt, name, down);
      });

      butt->activate();

      butt->setVisible(true);

    } else {
      butt->setVisible(false);
      butt->deactivate();
    }
    i++;
  }

  _node->setVisible(true);
  _node->doLayout();
}

void VoteForLeaderScene::update() {
  for (int player_id :
       _voting_info->votes[_player_controller->getMyPlayer()->getPlayerId()]) {
    CULog("%d", player_id);
  }
}

void VoteForLeaderScene::buttonListener(
    const std::shared_ptr<cugl::scene2::Button>& butt, const std::string& name,
    bool down) {
  if (!down) return;

  int voted_for = std::stoi(name);

  std::vector<int>& votes =
      _voting_info->votes[_player_controller->getMyPlayer()->getPlayerId()];

  auto it = std::find(votes.begin(), votes.end(), voted_for);
  if (it == votes.end()) {
    votes.push_back(voted_for);
  } else {
    votes.erase(it);
  }
}
