#include "Corrupt.h"

Corrupt::Corrupt()
    : _curr_down(false),
      _prev_down(false),
      _button(nullptr) {}

bool Corrupt::init(const std::shared_ptr<cugl::AssetManager> &assets,
                  cugl::Rect bounds) {
  Action::init(assets, bounds);

  _right_screen_bounds = Action::_display_coord_bounds;
  _right_screen_bounds.size.width *= 0.5f;
  _right_screen_bounds.origin.x += _right_screen_bounds.size.width;

  _button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      assets->get<cugl::scene2::SceneNode>("ui-scene_enrage"));

  _butt_down = false;
  _button->addListener([=](const std::string &name, bool down) {
    if (!_butt_down && down) {
      // If previously not down and button pressed. (i.e. first time down).
      _time_down_start.mark();
    }

    _butt_down = down;
  });

  _button->activate();

  return true;
}

bool Corrupt::update() {
  _prev_down = _curr_down;
  _curr_down = _butt_down;
  
  return true;
}

bool Corrupt::dispose() {
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
