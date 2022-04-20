#include "Dash.h"

Dash::Dash()
    : _curr_down(false),
      _prev_down(false),
      _button(nullptr),
      _dash_frames(12),
      _dash_frame_counter(0),
      _dash_cooldown(60),
      _dash_cooldown_counter(-1) {}

bool Dash::init(const std::shared_ptr<cugl::AssetManager> &assets,
                cugl::Rect bounds) {
  Action::init(assets, bounds);

  _button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      assets->get<cugl::scene2::SceneNode>("ui-scene_dash"));

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

bool Dash::update() {
  if (_butt_down) _button->setColor(cugl::Color4::GRAY);
  _prev_down = _curr_down;
  _curr_down = _butt_down;
  // Increment counter if dash button was just released or dash frames are still
  // occuring
  if ((_prev_down && !_curr_down) || _dash_frame_counter > 0) {
    _dash_frame_counter++;
    if (_dash_frame_counter > _dash_frames) {
      _dash_frame_counter = 0;
      _dash_cooldown_counter = 0;
    }
  }

  if (_dash_cooldown_counter >= 0) {
    setActive(false);
    _dash_cooldown_counter++;
    if (_dash_cooldown_counter > _dash_cooldown) {
      setActive(true);
      _button->setColor(cugl::Color4::WHITE);
      _dash_cooldown_counter = -1;
    }
  }
  return true;
}

bool Dash::dispose() {
  _button = nullptr;

#ifdef CU_TOUCH_SCREEN
  cugl::Touchscreen *touch = cugl::Input::get<cugl::Touchscreen>();
  touch->removeMotionListener(_listener_key);
#endif

  return true;
}

void Dash::setActive(bool value) {
  (value) ? _button->activate() : _button->deactivate();
}

#ifdef CU_TOUCH_SCREEN

void Dash::touchMoved(const cugl::TouchEvent &event, const cugl::Vec2 &previous,
                      bool focus) {
  if (_button->getTouchIds().find(event.touch) !=
      _button->getTouchIds().end()) {
  }
}
#endif  // CU_TOUCH_SCREEN
