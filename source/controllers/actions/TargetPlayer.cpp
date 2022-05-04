#include "TargetPlayer.h"

#define COOLDOWN 3600 // 1 minute cooldown to use block

TargetPlayer::TargetPlayer()
    : _curr_down(false),
      _prev_down(false),
      _button(nullptr),
      _target_player_hang_frames(150),
      _target_player_counter(0),
      _target_player_id(-1),
      _target_cooldown_counter(-1) {}

bool TargetPlayer::init(const std::shared_ptr<cugl::AssetManager> &assets,
                cugl::Rect bounds) {
  Action::init(assets, bounds);

  _button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      assets->get<cugl::scene2::SceneNode>("ui-scene_target-player"));

  _button->addListener(
      [=](const std::string &name, bool down) { _butt_down = down; });

  _button->activate();

#ifdef CU_TOUCH_SCREEN
  cugl::Touchscreen *touch = cugl::Input::get<cugl::Touchscreen>();
  _listener_key = touch->acquireKey();

  touch->addMotionListener(
      _listener_key,
      [=](const cugl::TouchEvent &event, const cugl::Vec2 &previous,
          bool focus) { this->touchMoved(event, previous, focus); });
#endif  // CU_TOUCH_SCREEN

  return true;
}

bool TargetPlayer::update() {
  _prev_down = _curr_down;
  _curr_down = _butt_down;
  // Increment counter if target player button was just released or target player frames
  // are still occuring
  
  if (_target_cooldown_counter <= 0) {
    setActive(true);
    if (isActivatingTargetAction()) {
      _is_activating_action = false;
      _target_player_counter = 0;
      _target_player_id = -1;
      _target_cooldown_counter = COOLDOWN;
      setActive(false);
    }
    if (didChangeTarget()) {
      _target_player_counter = 1;
    }
    if (_target_player_counter > 0) {
      _target_player_counter++;
      if (_target_player_counter > _target_player_hang_frames) {
        if (_target_player_id != -1) {
          _is_activating_action = true;
        } else {
          _target_player_counter = 0;
          _target_player_id = -1;
        }
      }
    }
  }
  _target_cooldown_counter--;
  return true;
}

bool TargetPlayer::dispose() {
  _button = nullptr;

#ifdef CU_TOUCH_SCREEN
  cugl::Touchscreen *touch = cugl::Input::get<cugl::Touchscreen>();
  touch->removeMotionListener(_listener_key);
#endif

  return true;
}

void TargetPlayer::setActive(bool value) {
  if (value) {
    _button->activate();
    _button->setColor(cugl::Color4::WHITE);
  } else {
    _button->deactivate();
    _button->setColor(cugl::Color4::GRAY);
  }
}

#ifdef CU_TOUCH_SCREEN

void TargetPlayer::touchMoved(const cugl::TouchEvent &event, const cugl::Vec2 &previous,
                      bool focus) {
  if (_button->getTouchIds().find(event.touch) !=
      _button->getTouchIds().end()) {
  }
}
#endif  // CU_TOUCH_SCREEN
