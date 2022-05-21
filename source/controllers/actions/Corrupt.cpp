#include "Corrupt.h"

#define CORRUPT_ANIM_LIMIT 25
#define CORRUPT_ANIM_LENGTH 10000 /* ms (i.e. cooldown) */

Corrupt::Corrupt() : _curr_down(false), _prev_down(false), _button(nullptr) {}

bool Corrupt::init(const std::shared_ptr<cugl::AssetManager> &assets,
                   cugl::Rect bounds) {
  _button_node = cugl::scene2::SpriteNode::alloc(
      assets->get<cugl::Texture>("infect-player"), 5, 6, CORRUPT_ANIM_LIMIT);
  _button_node->setFrame(0);
  _button = cugl::scene2::Button::alloc(_button_node);
  _button->setPlayClickSound(false);

  assets->get<cugl::scene2::SceneNode>("ui-scene_infect-player")
      ->addChild(_button);
  _button->setAnchor(cugl::Vec2::ANCHOR_CENTER);
  _button->setPosition(_button->getParent()->getContentSize() / 2);
  _button->setScale(_button->getParent()->getScale());
  _button->getParent()->doLayout();

  _anim_buffer = 0;
  _start_cooldown = false;

  _butt_down = false;
  _button->addListener(
      [=](const std::string &name, bool down) { _butt_down = down; });

  _button->activate();

  return true;
}

bool Corrupt::update() {
  _prev_down = _curr_down;
  _curr_down = _butt_down;

  // Increment counter if corrupt button was just released and cooldown has
  // stopped
  bool released = (_prev_down && !_curr_down && !_start_cooldown);
  if (released) {
    _start_cooldown = true;
    _button->deactivate();
    _time_cooldown_start.mark();
  }

  if (_start_cooldown) {
    cugl::Timestamp time;
    Uint64 millis = time.ellapsedMillis(_time_cooldown_start);
    int frame = _button_node->getFrame();

    if (frame < CORRUPT_ANIM_LIMIT - 1) {
      float diff = std::min(1.0f, millis * 1.0f / CORRUPT_ANIM_LENGTH);
      _button_node->setFrame((CORRUPT_ANIM_LIMIT - 1) * diff);
    } else if (frame == CORRUPT_ANIM_LIMIT - 1) {
      _start_cooldown = false;
      _button->resetDownColor();
    }
    _anim_buffer++;
  } else {
    _button_node->setFrame(0);
    _button->activate();
  }

  return true;
}

bool Corrupt::dispose() {
  auto parent = _button->getParent();
  if (parent) parent->removeChild(_button);
  _button = nullptr;

  return true;
}

void Corrupt::setActive(bool value) {
  if (value) {
    _button->activate();
    _button->setColor(cugl::Color4::WHITE);
  } else {
    _button->deactivate();
    _button->setColor(cugl::Color4::GRAY);
  }
}
