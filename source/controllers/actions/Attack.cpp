#include "Attack.h"

#define JOYSTICK_RADIUS 30.0f
#define JOYSTICK_DIFF_MIN 15.0f

#define TIME_TO_WAIT_FOR_CHARGE 200 /* ms */
#define CHARGE_ANIM_LIMIT 21
#define CHARGE_ANIM_LENGTH 700 /* ms */

Attack::Attack()
    : _anim_buffer(0),
      _charge_over(false),
      _curr_down(false),
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

  _button_node = cugl::scene2::SpriteNode::alloc(
      assets->get<cugl::Texture>("attack"), 4, 6, CHARGE_ANIM_LIMIT);
  _button_node->setFrame(0);
  _button = cugl::scene2::Button::alloc(_button_node);

  assets->get<cugl::scene2::SceneNode>("ui-scene_attack")->addChild(_button);
  _button->setAnchor(cugl::Vec2::ANCHOR_CENTER);
  _button->setPosition(_button->getParent()->getContentSize() / 2);
  _button->setScale(_button->getParent()->getScale());
  _button->getParent()->doLayout();

  _attack_base = std::dynamic_pointer_cast<cugl::scene2::PolygonNode>(
      assets->get<cugl::scene2::SceneNode>("ui-scene_attack-base"));

  _attack_base->setPosition(_button->getParent()->getPosition());
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
      this->_button->getParent()->setPosition(_joystick_anchor);
      this->_show_joystick_base = false;
    }
  });

  _button->activate();

  if (_joystick_on) {
    _joystick_anchor = _button->getParent()->getPosition();
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

  if (!_prev_down && _butt_down) {
    _time_down_start.mark();
  }

  if (_joystick_on) {
    if (_joystick_diff.length() > JOYSTICK_DIFF_MIN) {
      _show_joystick_base = true;
    }

    _attack_base->setVisible(_show_joystick_base);

    if (_show_joystick_base) {
      _button->getParent()->setPosition(_joystick_anchor + _joystick_diff);
    }
  }

  if (!_show_joystick_base && _butt_down) {
    cugl::Timestamp time;
    Uint64 millis = time.ellapsedMillis(_time_down_start);
    int frame = _button_node->getFrame();

    // Wait a bit before charing, so that it doesn't charge on quick tap.
    if (millis <= TIME_TO_WAIT_FOR_CHARGE) {
      millis = 0;
    } else {
      // Reset millis so that animation starts at frame 0
      millis -= TIME_TO_WAIT_FOR_CHARGE;
      if (!_charge_start && !_charge_running) _charge_start = true;
      _charge_running = true;
    }

    if (frame < CHARGE_ANIM_LIMIT - 1) {
      float diff = std::min(1.0f, millis * 1.0f / CHARGE_ANIM_LENGTH);
      _button_node->setFrame((CHARGE_ANIM_LIMIT - 1) * diff);
    } else if (frame == CHARGE_ANIM_LIMIT - 1) {
      _charge_over = true;
    }
    _anim_buffer++;
  } else {
    _button_node->setFrame(0);
    _charge_over = false;
    _charge_running = false;
    _charge_start = false;
  }

  return true;
}

bool Attack::dispose() {
  _button = nullptr;
  _attack_base = nullptr;
  _charge_over = false;

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
