#include "Attack.h"

#define JOYSTICK_RADIUS 30.0f
#define TIME_TO_WAIT_FOR_JOYSTICK 40

Attack::Attack()
    : _curr_down(false),
      _prev_down(false),
      _button(nullptr),
      _attack_base(nullptr),
      _show_joystick_base(false) {}

bool Attack::init(const std::shared_ptr<cugl::AssetManager> &assets,
                  cugl::Rect bounds) {
  Action::init(assets, bounds);
  // !IMPORTANT! If joystick should be used, set this to true.
  _joystick_on = false;

  _right_screen_bounds = Action::_display_coord_bounds;
  _right_screen_bounds.size.width *= 0.5f;
  _right_screen_bounds.origin.x += _right_screen_bounds.size.width;

  _button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      assets->get<cugl::scene2::SceneNode>("ui-scene_attack"));

  _attack_base = std::dynamic_pointer_cast<cugl::scene2::PolygonNode>(
      assets->get<cugl::scene2::SceneNode>("ui-scene_attack-base"));

  _attack_base->setPosition(_button->getPosition());
  _attack_base->setVisible(false);

  _butt_down = false;
  _button->addListener([=](const std::string &name, bool down) {
    if (!_butt_down && down) {
      // If previously not down and button pressed. (i.e. first time down).
      _time_down_start.mark();
    }

    _butt_down = down;

    if (!down && _joystick_on) {
      this->_joystick_diff.set(0, 0);
      this->_show_joystick_base = false;
    }
  });

  _button->activate();

  if (_joystick_on) {
    _joystick_anchor = _button->getPosition();
  }

#ifdef CU_TOUCH_SCREEN
  if (_joystick_on) {
    cugl::Touchscreen *touch = cugl::Input::get<cugl::Touchscreen>();
    _listener_key = touch->acquireKey();

    touch->addMotionListener(
        _listener_key,
        [=](const cugl::TouchEvent &event, const cugl::Vec2 &previous,
            bool focus) { this->touchMoved(event, previous, focus); });
  }
#endif  // CU_TOUCH_SCREEN

  return true;
}

bool Attack::update() {
  _prev_down = _curr_down;
  _curr_down = _butt_down;

  if (_joystick_on) {
    if (_joystick_diff.length() > 0.1f) _show_joystick_base = true;

    _attack_base->setVisible(_show_joystick_base);
    _button->setPosition(_joystick_anchor + _joystick_diff);
  }

  return true;
}

bool Attack::dispose() {
  _button = nullptr;
  _attack_base = nullptr;

#ifdef CU_TOUCH_SCREEN
  if (_joystick_on) {
    cugl::Touchscreen *touch = cugl::Input::get<cugl::Touchscreen>();
    touch->removeMotionListener(_listener_key);
  }
#endif

  return true;
}

void Attack::setActive(bool value) {
  (value) ? _button->activate() : _button->deactivate();
}

#ifdef CU_TOUCH_SCREEN

void Attack::touchMoved(const cugl::TouchEvent &event,
                        const cugl::Vec2 &previous, bool focus) {
  if (_joystick_on && _button->getTouchIds().find(event.touch) !=
                          _button->getTouchIds().end()) {
    _joystick_diff =
        Action::displayToScreenCoord(event.position).subtract(_joystick_anchor);

    float clamped_diff_len =
        clampf(_joystick_diff.length(), 0, JOYSTICK_RADIUS);

    _joystick_diff.normalize().scale(clamped_diff_len);
  }
}
#endif  // CU_TOUCH_SCREEN
