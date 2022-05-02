#include "Dash.h"

#define DASH_ANIM_LIMIT 30
#define DASH_ANIM_LENGTH 1000 /* ms */

Dash::Dash()
    : _curr_down(false),
      _prev_down(false),
      _button(nullptr),
      _start_cooldown(false),
      _anim_buffer(0),
      _dash_frames(12),
      _dash_frame_counter(0),
      _dash_cooldown(60) {}

bool Dash::init(const std::shared_ptr<cugl::AssetManager> &assets,
                cugl::Rect bounds) {
  Action::init(assets, bounds);

  _button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      assets->get<cugl::scene2::SceneNode>("ui-scene_dash"));

  _button_node = cugl::scene2::SpriteNode::alloc(
      assets->get<cugl::Texture>("dash"), 5, 6, DASH_ANIM_LIMIT);
  _button_node->setFrame(0);
  _button = cugl::scene2::Button::alloc(_button_node);

  _start_cooldown = false;

  _anim_buffer = 0;

  assets->get<cugl::scene2::SceneNode>("ui-scene_dash")->addChild(_button);
  _button->setAnchor(cugl::Vec2::ANCHOR_CENTER);
  _button->setPosition(_button->getParent()->getContentSize() / 2);
  _button->setScale(_button->getParent()->getScale());
  _button->getParent()->doLayout();

  _button->addListener([=](const std::string &name, bool down) {
    _butt_down = down;
    if (!_butt_down) {
      _button->setDownColor(cugl::Color4::WHITE);
    }
  });

  _button->activate();

#ifdef CU_TOUCH_SCREEN
  cugl::Touchscreen *touch = cugl::Input::get<cugl::Touchscreen>();
  _listener_key = touch->acquireKey();

  touch->addMotionListener(
      _listener_key,
      [=](const cugl::TouchEventa &event, const cugl::Vec2 &previous,
          bool focus) { this->touchMoved(event, previous, focus); });
#endif  // CU_TOUCH_SCREEN

  return true;
}

bool Dash::update() {
  _prev_down = _curr_down;
  _curr_down = _butt_down;

  // Increment counter if dash button was just released and cooldown has stopped
  bool released = (_prev_down && !_curr_down && !_start_cooldown);
  if (released || _dash_frame_counter > 0) {
    _dash_frame_counter++;
    if (_dash_frame_counter > _dash_frames) {
      _dash_frame_counter = 0;
      _start_cooldown = true;
      _time_cooldown_start.mark();
    }
  }

  if (_start_cooldown) {
    cugl::Timestamp time;
    Uint64 millis = time.ellapsedMillis(_time_cooldown_start);
    int frame = _button_node->getFrame();

    if (frame < DASH_ANIM_LIMIT - 1) {
      float diff = std::min(1.0f, millis * 1.0f / DASH_ANIM_LENGTH);
      _button_node->setFrame((DASH_ANIM_LIMIT - 1) * diff);
    } else if (frame == DASH_ANIM_LIMIT - 1) {
      _start_cooldown = false;
      _button->resetDownColor();
    }
    _anim_buffer++;
  } else {
    _button_node->setFrame(0);
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
