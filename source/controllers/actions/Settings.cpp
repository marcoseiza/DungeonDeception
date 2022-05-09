#include "Settings.h"

Settings::Settings() : _curr_down(false), _prev_down(false), _button(nullptr) {}

bool Settings::init(const std::shared_ptr<cugl::AssetManager> &assets,
                    cugl::Rect bounds) {
  Action::init(assets, bounds);

  _button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      assets->get<cugl::scene2::SceneNode>("ui-scene_settings"));

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

bool Settings::update() {
  _prev_down = _curr_down;
  _curr_down = _butt_down;
  return true;
}

bool Settings::dispose() {
  _button = nullptr;

#ifdef CU_TOUCH_SCREEN
  cugl::Touchscreen *touch = cugl::Input::get<cugl::Touchscreen>();
  touch->removeMotionListener(_listener_key);
#endif

  return true;
}

void Settings::setActive(bool value) {
  (value) ? _button->activate() : _button->deactivate();
}

#ifdef CU_TOUCH_SCREEN

void Settings::touchMoved(const cugl::TouchEvent &event,
                          const cugl::Vec2 &previous, bool focus) {
  if (_button->getTouchIds().find(event.touch) !=
      _button->getTouchIds().end()) {
  }
}
#endif  // CU_TOUCH_SCREEN
