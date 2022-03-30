#include "VoteForLeaderScene.h"

#define NODE_NAME "terminal-voting-scene_voting-background_vote-for-leader"

bool VoteForLeaderScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;

  _node = _assets->get<cugl::scene2::SceneNode>(NODE_NAME);
  _node->setVisible(false);

  _initialized = true;
  return true;
}

void VoteForLeaderScene::start(std::shared_ptr<VotingInfo> voting_info) {
  if (!_initialized || _active) return;
  _active = true;

  _voting_info = voting_info;

  auto buttons_layout = cugl::scene2::GridLayout::alloc();
  int num_players = _voting_info->players.size();
  buttons_layout->setGridSize(cugl::Size(2, (int)(num_players - 1 / 2) + 1));
  auto buttons = cugl::scene2::SceneNode::alloc();
  buttons->setContentSize(cugl::Size(600, 75 * (int)(num_players - 1 / 2) + 1));
  buttons->setLayout(buttons_layout);

  for (int i = 0; i < _voting_info->players.size(); i++) {
    int player_id = _voting_info->players[i];
    std::string name = "player " + std::to_string(player_id);
    auto butt_info =
        _assets->get<cugl::JsonValue>("terminal-voting-button-use");
    butt_info->setKey(NODE_NAME + name);
    auto vars = butt_info->get("data")->get("variables");
    vars->get("text")->set(name);
    _assets->load<cugl::scene2::SceneNode>(
        NODE_NAME + name, "widgets/terminal-voting-button-use.json");
    auto button = _assets->get<cugl::scene2::SceneNode>(NODE_NAME + name);
    button->setName(name);
    buttons_layout->addPosition(name, i % 2, (int)(i / 2),
                                cugl::scene2::Layout::Anchor::CENTER);
    buttons->addChild(button);
  }

  _node->setVisible(true);
  _node->doLayout();
}

void VoteForLeaderScene::update() {}