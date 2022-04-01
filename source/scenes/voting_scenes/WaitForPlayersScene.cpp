#include "WaitForPlayersScene.h"

#define WAIT_TIME_AFTER_REQUIRED_ACCOMPLISHED 600

bool WaitForPlayersScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;

  _node = _assets->get<cugl::scene2::SceneNode>(
      "terminal-voting-scene_voting-background_wait-for-player");
  _node->setVisible(false);

  _initialized = true;
  return true;
}

void WaitForPlayersScene::start(int num_players_req) {
  if (!_initialized || _active) return;
  _active = true;
  _node->setVisible(true);

  _num_players_req = num_players_req;
  auto num_required_label = std::dynamic_pointer_cast<cugl::scene2::Label>(
      _node->getChildByName("num-required"));
  num_required_label->setText(
      "Number Of People Required: " + std::to_string(num_players_req), true);

  _node->doLayout();
}

void WaitForPlayersScene::update() {
  if (_curr_num_players >= _num_players_req) _buffer_time_after_required++;
  if (_buffer_time_after_required > WAIT_TIME_AFTER_REQUIRED_ACCOMPLISHED) {
    _done = true;  // DONE!
    return;
  }

  auto current_num_label = std::dynamic_pointer_cast<cugl::scene2::Label>(
      _node->getChildByName("current-num"));

  current_num_label->setText(
      "Current Number Of Players: " + std::to_string(_curr_num_players), true);
}