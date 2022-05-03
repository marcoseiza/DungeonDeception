#include "Movement.h"

#define LEFT_ZONE_FRAC 0.5f

#define JOYSTICK_RADIUS 30.0f

Movement::Movement()
    : _show_joystick(false),
      _joystick_base(nullptr),
      _joystick(nullptr),
      _listener_key(0) {}

bool Movement::init(const std::shared_ptr<cugl::AssetManager> &assets,
                    cugl::Rect bounds) {
  Action::init(assets, bounds);
  _left_screen_bounds = Action::_display_coord_bounds;
  _left_screen_bounds.size.width *= LEFT_ZONE_FRAC;

  _pause = false;

  _joystick_base = std::dynamic_pointer_cast<cugl::scene2::PolygonNode>(
      assets->get<cugl::scene2::SceneNode>("ui-scene_joystick-base"));
  _joystick = std::dynamic_pointer_cast<cugl::scene2::PolygonNode>(
      assets->get<cugl::scene2::SceneNode>("ui-scene_joystick"));

#ifdef CU_TOUCH_SCREEN
  cugl::Touchscreen *touch = cugl::Input::get<cugl::Touchscreen>();
  _listener_key = touch->acquireKey();

  touch->addBeginListener(_listener_key,
                          [=](const cugl::TouchEvent &event, bool focus) {
                            this->touchBegan(event, focus);
                          });

  touch->addEndListener(_listener_key,
                        [=](const cugl::TouchEvent &event, bool focus) {
                          this->touchEnded(event, focus);
                        });

  touch->addMotionListener(
      _listener_key,
      [=](const cugl::TouchEvent &event, const cugl::Vec2 &previous,
          bool focus) { this->touchMoved(event, previous, focus); });
#endif  // CU_TOUCH_SCREEN

  return true;
}

bool Movement::update() {
  if (_pause) return true;

  _joystick->setVisible(_show_joystick);
  _joystick_base->setVisible(_show_joystick);

  _joystick_base->setPosition(_joystick_anchor);
  _joystick->setPosition(_joystick_diff + _joystick_anchor);

#ifndef CU_TOUCH_SCREEN
  _joystick_diff.setZero();

  cugl::Keyboard *keyboard = cugl::Input::get<cugl::Keyboard>();
  if (keyboard->keyDown(cugl::KeyCode::D))
    _joystick_diff.x = 1.0f;
  else if (keyboard->keyDown(cugl::KeyCode::A))
    _joystick_diff.x = -1.0f;

  if (keyboard->keyDown(cugl::KeyCode::W))
    _joystick_diff.y = 1.0f;
  else if (keyboard->keyDown(cugl::KeyCode::S))
    _joystick_diff.y = -1.0f;
#endif

  return true;
}

bool Movement::dispose() {
  _joystick = nullptr;
  _joystick_base = nullptr;

#ifdef CU_TOUCH_SCREEN
  cugl::Touchscreen *touch = cugl::Input::get<cugl::Touchscreen>();
  touch->removeBeginListener(_listener_key);
  touch->removeEndListener(_listener_key);
  touch->removeMotionListener(_listener_key);
#endif

  return true;
}

void Movement::pause() {
  _pause = true;
  _joystick->setVisible(false);
  _joystick_base->setVisible(false);
  _joystick_diff.setZero();
}

void Movement::resume() { _pause = false; }

#pragma mark Listeners

#ifdef CU_TOUCH_SCREEN

void Movement::touchBegan(const cugl::TouchEvent &event, bool focus) {
  if (_pause) return;

  cugl::Vec2 pos = event.position;

  if (_left_screen_bounds.contains(pos) && _touch_ids.empty()) {
    _touch_ids.insert(event.touch);

    _show_joystick = true;
    _joystick_anchor = Action::displayToScreenCoord(pos);
  }
}

void Movement::touchEnded(const cugl::TouchEvent &event, bool focus) {
  if (_pause) {
    _show_joystick = false;
    _touch_ids.clear();
    _joystick_diff.setZero();
  }

  if (_touch_ids.find(event.touch) != _touch_ids.end()) {
    _show_joystick = false;
    _touch_ids.clear();
    _joystick_diff.setZero();
  }
}

void Movement::touchMoved(const cugl::TouchEvent &event,
                          const cugl::Vec2 &previous, bool focus) {
  if (_pause) {
    _joystick_diff.setZero();
  }

  if (_touch_ids.find(event.touch) != _touch_ids.end()) {
    cugl::Vec2 pos = event.position;

    _joystick_diff =
        Action::displayToScreenCoord(pos).subtract(_joystick_anchor);

    float clamped_diff_len =
        clampf(_joystick_diff.length(), 0, JOYSTICK_RADIUS);

    _joystick_diff.normalize().scale(clamped_diff_len);
  }
}
#endif  // CU_TOUCH_SCREEN
