#include "VoteForLeaderScene.h"

bool VoteForLeaderScene::init(const std::shared_ptr<cugl::AssetManager>& assets,
                              int my_player_id) {
  _assets = assets;

  _node = _assets->get<cugl::scene2::SceneNode>(
      "terminal-voting-scene_voting-background_vote-for-leader");
  // _node->setVisible(false);

  _my_player_id = my_player_id;
  _initialized = true;
  return true;
}

void VoteForLeaderScene::start(std::shared_ptr<VotingInfo> voting_info) {
  if (!_initialized || _active) return;
  _active = true;

  _voting_info = voting_info;

  int num_players = _voting_info->players.size();

  for (int i = 0; i < _voting_info->players.size(); i++) {
    int player_id = _voting_info->players[i];

    std::string label_name = cugl::strtool::format(
        "terminal-voting-scene_voting-background_vote-for-leader_buttons_"
        "button-wrapper-%d_button-%d_up_label",
        i, i);
    auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
        _assets->get<cugl::scene2::SceneNode>(label_name));

    std::string name = "player " + std::to_string(player_id);
    label->setText(name, true);

    std::string button_name = cugl::strtool::format(
        "terminal-voting-scene_voting-background_vote-for-leader_buttons_"
        "button-wrapper-%d_button-%d",
        i, i);
    auto button = std::dynamic_pointer_cast<cugl::scene2::Button>(
        _assets->get<cugl::scene2::SceneNode>(button_name));

    button->setName(std::to_string(player_id));

    button->addListener([=](const std::string& name, bool down) {
      this->buttonListener(name, down);
    })
  }

  _node->setVisible(true);
  _node->doLayout();
}

void VoteForLeaderScene::update() {}

void VoteForLeaderScene::buttonListener(const std::string& name, bool down) {
  int voted_for = std::stoi(name);
  _voting_info->votes[]
}
